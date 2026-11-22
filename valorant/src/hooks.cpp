// hook pipeline: VEH + PAGE_GUARD on IDXGISwapChain::Present.
// no bytes patched — Vanguard's Present prologue hash stays intact.
// render thread executes:
//   1. refresh entity snapshot from GWorld
//   2. update visibility flags
//   3. run aimbot + triggerbot
//   4. draw ESP + menu via ImGui background drawlist
//   5. jump back to Present original flow via single-step continue
//
// deliberately no SetWindowsHookEx / SetWindowLongPtr — both are scanned.
// input for ImGui comes from GetAsyncKeyState polling, zero WndProc touch.

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <atomic>
#include <mutex>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui/backends/imgui_impl_win32.h"

#include "state.h"
#include "entities.h"
#include "visibility.h"
#include "esp.h"
#include "aim.h"
#include "trigger.h"
#include "menu.h"
#include "sdk.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace hooks {

    static uintptr_t         g_present_addr   = 0;
    static uintptr_t         g_present_page   = 0;
    static SIZE_T            g_present_pagesz = 0x1000;
    static PVOID             g_veh_handle     = nullptr;

    static ID3D11Device*         g_dev    = nullptr;
    static ID3D11DeviceContext*  g_ctx    = nullptr;
    static ID3D11RenderTargetView* g_rtv  = nullptr;
    static IDXGISwapChain*       g_last_sc= nullptr;
    static HWND                  g_hwnd   = nullptr;
    static bool                  g_imgui_ready = false;
    static std::atomic<bool>     g_in_hook { false };

    static void ensure_rtv(IDXGISwapChain* sc) {
        if (g_rtv) return;
        ID3D11Texture2D* bb = nullptr;
        if (SUCCEEDED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb)) && bb) {
            g_dev->CreateRenderTargetView(bb, nullptr, &g_rtv);
            bb->Release();
        }
    }

    static void init_imgui(IDXGISwapChain* sc) {
        if (g_imgui_ready) return;
        if (FAILED(sc->GetDevice(__uuidof(ID3D11Device), (void**)&g_dev))) return;
        g_dev->GetImmediateContext(&g_ctx);

        DXGI_SWAP_CHAIN_DESC scd{};
        sc->GetDesc(&scd);
        g_hwnd = scd.OutputWindow;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.IniFilename = nullptr;
        ImGui::StyleColorsDark();
        auto& s = ImGui::GetStyle();
        s.WindowRounding = 4.0f; s.FrameRounding = 3.0f;
        s.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.08f, 0.94f);

        ImGui_ImplWin32_Init(g_hwnd);
        ImGui_ImplDX11_Init(g_dev, g_ctx);
        ensure_rtv(sc);
        g_imgui_ready = true;
    }

    // minimal mouse/keyboard poll — feeds ImGui without a WndProc hook
    static void feed_input() {
        ImGuiIO& io = ImGui::GetIO();
        POINT p{};
        GetCursorPos(&p);
        ScreenToClient(g_hwnd, &p);
        RECT r{};
        GetClientRect(g_hwnd, &r);
        io.DisplaySize = ImVec2((float)(r.right - r.left), (float)(r.bottom - r.top));
        io.MousePos    = ImVec2((float)p.x, (float)p.y);
        io.MouseDown[0]= (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        io.MouseDown[1]= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        io.MouseDown[2]= (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
    }

    static void do_frame(IDXGISwapChain* sc) {
        if (!g_imgui_ready) init_imgui(sc);
        if (!g_imgui_ready) return;
        ensure_rtv(sc);

        feed_input();

        // snapshot world
        uintptr_t w  = sdk::gworld();
        uintptr_t gi = sdk::game_instance(w);
        uintptr_t lp = sdk::local_player(gi);
        uintptr_t pc = sdk::player_controller(lp);
        uintptr_t me = sdk::local_pawn(pc);

        ents::refresh(pc, me);
        vis::update();
        aim::tick(pc);

        ImGuiIO& io = ImGui::GetIO();
        trigger::tick(pc, io.DisplaySize.x, io.DisplaySize.y);

        // render
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        esp::render();
        menu::render();

        ImGui::EndFrame();
        ImGui::Render();
        g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    // VEH — called on guard-page hit and on the single-step trap that follows.
    static LONG CALLBACK veh(EXCEPTION_POINTERS* ex) {
        auto* er = ex->ExceptionRecord;

        if (er->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION &&
            er->ExceptionInformation[1] == g_present_addr) {

            // read-side arg recovery: fastcall rcx = this ptr = swap chain
            auto* sc = reinterpret_cast<IDXGISwapChain*>(ex->ContextRecord->Rcx);
            g_last_sc = sc;

            if (!g_in_hook.exchange(true)) {
                __try { do_frame(sc); } __except(1) { }
                g_in_hook = false;
            }

            // enable TF — the kernel already cleared PAGE_GUARD on this hit;
            // single-step will refire veh so we can re-arm.
            ex->ContextRecord->EFlags |= 0x100;
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        if (er->ExceptionCode == STATUS_SINGLE_STEP) {
            // re-arm guard on Present page
            DWORD old;
            VirtualProtect((LPVOID)g_present_page, g_present_pagesz,
                           PAGE_EXECUTE_READ | PAGE_GUARD, &old);
            ex->ContextRecord->EFlags &= ~0x100;
            return EXCEPTION_CONTINUE_EXECUTION;
        }

        return EXCEPTION_CONTINUE_SEARCH;
    }

    // dummy swap chain → lift Present vptr. the real game instance will share
    // the same vtable (per-process d3d11 vtable is unique but stable).
    static uintptr_t resolve_present_vptr() {
        HWND hwnd = CreateWindowExA(0, "static", "", WS_OVERLAPPEDWINDOW,
                                    0, 0, 2, 2, nullptr, nullptr,
                                    GetModuleHandleA(nullptr), nullptr);
        if (!hwnd) return 0;

        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount = 1;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hwnd;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0, flout;
        ID3D11Device* dev = nullptr; ID3D11DeviceContext* ctx = nullptr; IDXGISwapChain* sc = nullptr;

        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
            &fl, 1, D3D11_SDK_VERSION,
            &scd, &sc, &dev, &flout, &ctx);

        uintptr_t out = 0;
        if (SUCCEEDED(hr) && sc) {
            void** vtbl = *reinterpret_cast<void***>(sc);
            out = reinterpret_cast<uintptr_t>(vtbl[8]); // Present
        }
        if (sc)  sc->Release();
        if (ctx) ctx->Release();
        if (dev) dev->Release();
        DestroyWindow(hwnd);
        return out;
    }

    bool install() {
        g_present_addr = resolve_present_vptr();
        if (!g_present_addr) return false;

        g_present_page = g_present_addr & ~(SIZE_T)0xFFF;
        SYSTEM_INFO si; GetSystemInfo(&si);
        g_present_pagesz = si.dwPageSize;

        g_veh_handle = AddVectoredExceptionHandler(1, veh);
        if (!g_veh_handle) return false;

        DWORD old;
        if (!VirtualProtect((LPVOID)g_present_page, g_present_pagesz,
                            PAGE_EXECUTE_READ | PAGE_GUARD, &old)) {
            RemoveVectoredExceptionHandler(g_veh_handle);
            g_veh_handle = nullptr;
            return false;
        }
        return true;
    }

    void uninstall() {
        DWORD old;
        if (g_present_page)
            VirtualProtect((LPVOID)g_present_page, g_present_pagesz,
                           PAGE_EXECUTE_READ, &old);
        if (g_veh_handle) {
            RemoveVectoredExceptionHandler(g_veh_handle);
            g_veh_handle = nullptr;
        }
        if (g_imgui_ready) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            g_imgui_ready = false;
        }
        if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
        if (g_ctx) { g_ctx->Release(); g_ctx = nullptr; }
        if (g_dev) { g_dev->Release(); g_dev = nullptr; }
    }
}
