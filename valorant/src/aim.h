// aimbot — writes to APlayerController::ControlRotation, server mirrors it,
// so no silent aim. humanized interpolation + fov gate to stay off trivial
// behavioural detection (instant 180s, zero-variance deltas).

#pragma once
#include <Windows.h>
#include <cmath>
#include "sdk.h"
#include "entities.h"
#include "state.h"
#include "w2s.h"

namespace aim {

    struct Target {
        bool            valid = false;
        uintptr_t       pawn  = 0;
        sdk::FVector    point;
        float           score = 1e9f;
    };

    inline sdk::FRotator rot_to(const sdk::FVector& from, const sdk::FVector& to) {
        sdk::FVector d = to - from;
        float dist2d = d.size2d();
        float r2d = 180.0f / 3.14159265358979f;
        sdk::FRotator r;
        r.pitch = std::atan2(d.z, dist2d) * r2d;
        r.yaw   = std::atan2(d.y, d.x)    * r2d;
        r.roll  = 0;
        return r;
    }

    inline float delta_fov(const sdk::FRotator& a, const sdk::FRotator& b) {
        auto n = sdk::FRotator{ a.pitch - b.pitch, a.yaw - b.yaw, 0 }.normalize();
        return std::sqrt(n.pitch*n.pitch + n.yaw*n.yaw);
    }

    inline Target pick(uintptr_t pc) {
        Target best;
        if (!state::cfg.aim_enabled) return best;
        if (!(GetAsyncKeyState(state::cfg.aim_key) & 0x8000)) return best;

        int bone = state::cfg.aim_bone;
        for (const auto& e : ents::frame) {
            if (e.downed) continue;

            sdk::FVector p = sdk::bone_world(e.pawn, bone);
            sdk::FRotator want = rot_to(ents::cam_pos, p);
            float fov = delta_fov(want, ents::cam_rot);
            if (fov > state::cfg.aim_fov) continue;

            if (state::cfg.aim_visonly && !e.visible) continue;

            if (fov < best.score) {
                best.score = fov;
                best.pawn  = e.pawn;
                best.point = p;
                best.valid = true;
            }
        }
        return best;
    }

    // humanize: easing curve with per-axis jitter, scaled by distance.
    // smooth=1 snaps (ban bait), smooth=10 ≈ 60ms track.
    inline sdk::FRotator step(const sdk::FRotator& cur, const sdk::FRotator& want) {
        float s = state::cfg.aim_smooth;
        if (s < 1.0f) s = 1.0f;

        auto d = sdk::FRotator{ want.pitch - cur.pitch, want.yaw - cur.yaw, 0 }.normalize();
        float mag = std::sqrt(d.pitch*d.pitch + d.yaw*d.yaw);
        // ease-in: closer target => softer step. prevents snap when near crosshair.
        float ease = 1.0f - std::exp(-mag * 0.12f);
        float k = ease / s;

        sdk::FRotator out = cur;
        out.pitch += d.pitch * k;
        out.yaw   += d.yaw   * k;

        if (state::cfg.aim_humanize) {
            // tiny gaussian-ish jitter via two uniforms.
            thread_local uint32_t seed = 0x1a2b3c4d;
            auto rng = [&]() {
                seed = seed * 1664525u + 1013904223u;
                return (float)(seed & 0xFFFF) / 65535.0f - 0.5f;
            };
            float j = mag * 0.02f;
            out.pitch += (rng() + rng()) * j;
            out.yaw   += (rng() + rng()) * j;
        }
        return out.normalize();
    }

    inline void tick(uintptr_t pc) {
        if (!pc) return;
        Target t = pick(pc);
        if (!t.valid) return;

        if (state::cfg.aim_ads_only) {
            uintptr_t w = sdk::read<uintptr_t>(pc + off::PC_CurrentWeapon);
            if (!w || !(sdk::read<uint8_t>(w + off::Wpn_bIsScoping) & 1)) return;
        }

        auto cur  = sdk::read<sdk::FRotator>(pc + off::APlayerController_ControlRotation);
        auto want = rot_to(ents::cam_pos, t.point);
        auto nxt  = step(cur, want);
        sdk::write<sdk::FRotator>(pc + off::APlayerController_ControlRotation, nxt);

        if (state::cfg.no_recoil) {
            uintptr_t w = sdk::read<uintptr_t>(pc + off::PC_CurrentWeapon);
            if (w) sdk::write<sdk::FVector2D>(w + off::Wpn_RecoilVector, { 0, 0 });
        }
    }
}
