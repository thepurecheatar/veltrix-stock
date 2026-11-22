// mapped entry. the loader (BYOVD → MiAllocateVirtualMemory → APC queue)
// jumps here with no DllMain, no CRT, no PE headers after the first line.
//
// anything that touches static init must not depend on CRT globals —
// compile with /GS- /GR- /EHa and set entry via linker /ENTRY:veltrix_entry.

#include <Windows.h>
#include <intrin.h>

namespace hooks { bool install(); void uninstall(); }

namespace sdk {
    static uintptr_t g_base = 0;
    uintptr_t game_base() { return g_base; }
}

// wipe our own PE header once we're parked. Vanguard's first module walk
// scans for a DOS stub in private+exec VADs; zero it before that lands.
static void wipe_header(HMODULE self) {
    if (!self) return;
    DWORD old;
    if (VirtualProtect(self, 0x1000, PAGE_READWRITE, &old)) {
        RtlSecureZeroMemory(self, 0x1000);
        VirtualProtect(self, 0x1000, old, &old);
    }
}

static DWORD WINAPI veltrix_thread(LPVOID) {
    // resolve base of the host process (VALORANT-Win64-Shipping.exe).
    // walking PEB for ImageBaseAddress at [gs:0x60] + 0x10 on x64.
#if defined(_M_X64)
    uintptr_t peb = __readgsqword(0x60);
    sdk::g_base = *reinterpret_cast<uintptr_t*>(peb + 0x10);
#else
    uintptr_t peb = __readfsdword(0x30);
    sdk::g_base = *reinterpret_cast<uintptr_t*>(peb + 0x08);
#endif
    if (!sdk::g_base) sdk::g_base = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));

    wipe_header(GetModuleHandleA("veltrix.dll")); // only if we got mapped as a named module
    // if we're raw shellcode this is a nop — hashes still wipe below

    if (!hooks::install()) return 1;

    while (true) {
        if (GetAsyncKeyState(VK_END) & 0x8000) break;
        Sleep(50);
    }

    hooks::uninstall();
    return 0;
}

// exported for the loader; also serves as DllMain in debug builds where the
// mapper fires DLL_PROCESS_ATTACH.
extern "C" __declspec(dllexport)
BOOL WINAPI DllMain(HINSTANCE h, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h);
        CreateThread(nullptr, 0, veltrix_thread, nullptr, 0, nullptr);
    }
    return TRUE;
}

// standalone entry for mapper-based loads (no PE entry runs; mapper calls this
// directly). returns to the trampoline, loader rewrites return site to a ret.
extern "C" __declspec(dllexport)
void __cdecl veltrix_entry() {
    CreateThread(nullptr, 0, veltrix_thread, nullptr, 0, nullptr);
}
