// triggerbot — crosshair -> enemy pawn test via FOV cone against bone cloud.
// not a line trace (LineTraceSingleByChannel hashes its callsite). instead we
// re-project every known enemy bone and fire if any lands inside a tiny
// pixel radius around screen center. equivalent fidelity, zero crossings
// into scanned engine functions.

#pragma once
#include <Windows.h>
#include <chrono>
#include "entities.h"
#include "state.h"
#include "w2s.h"

namespace trigger {

    inline std::chrono::steady_clock::time_point last_fire;

    inline void send_click_down() {
        INPUT in{};
        in.type = INPUT_MOUSE;
        in.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &in, sizeof(in));
    }
    inline void send_click_up() {
        INPUT in{};
        in.type = INPUT_MOUSE;
        in.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &in, sizeof(in));
    }

    inline void tick(uintptr_t pc, float sw, float sh) {
        if (!state::cfg.trig_enabled) return;
        if (!(GetAsyncKeyState(state::cfg.trig_key) & 0x8000)) return;

        if (state::cfg.trig_ads_only) {
            uintptr_t w = sdk::read<uintptr_t>(pc + off::PC_CurrentWeapon);
            if (!w || !(sdk::read<uint8_t>(w + off::Wpn_bIsScoping) & 1)) return;
        }

        const float cx = sw * 0.5f, cy = sh * 0.5f;
        const float R  = 3.5f; // px; tight enough that a visible enemy body under crosshair triggers it

        bool on_target = false;
        for (const auto& e : ents::frame) {
            if (e.downed || !e.visible) continue;

            static const int ids[] = {
                off::bone::Head, off::bone::Neck, off::bone::Chest, off::bone::Pelvis,
                off::bone::UpperArmL, off::bone::UpperArmR,
                off::bone::ThighL,    off::bone::ThighR,
            };
            for (int id : ids) {
                auto p = sdk::bone_world(e.pawn, id);
                auto s = w2s::project(p, ents::cam_pos, ents::cam_rot, ents::cam_fov, sw, sh);
                if (!s.on) continue;
                float dx = s.x - cx, dy = s.y - cy;
                if (dx*dx + dy*dy < R*R) { on_target = true; break; }
            }
            if (on_target) break;
        }

        if (!on_target) return;

        auto now = std::chrono::steady_clock::now();
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_fire).count();
        if (ms < state::cfg.trig_delay_ms + 60) return; // 60ms click cycle guard

        send_click_down();
        Sleep(25);
        send_click_up();
        last_fire = std::chrono::steady_clock::now();
    }
}
