// enemy walk + cached snapshot. rebuilt each frame on render thread.
// touching the game's actor array from multiple threads = Vanguard pattern hit,
// so we keep every read strictly on the Present-hook thread.

#pragma once
#include <vector>
#include "sdk.h"
#include "state.h"

namespace ents {

    struct Snapshot {
        uintptr_t pawn       = 0;
        uintptr_t ps         = 0;        // ShooterPlayerState
        int32_t   team       = -1;
        int32_t   hp         = 0;
        int32_t   hp_max     = 100;
        bool      downed     = false;
        bool      visible    = false;    // set by aim/trace, not here
        sdk::FVector head;
        sdk::FVector pelvis;
        sdk::FVector feet;
    };

    inline std::vector<Snapshot> frame;
    inline int32_t local_team = -1;
    inline sdk::FVector cam_pos;
    inline sdk::FRotator cam_rot;
    inline float cam_fov = 103.0f;

    inline void refresh(uintptr_t pc, uintptr_t local_pawn) {
        frame.clear();
        if (!pc) return;

        // camera
        uintptr_t cm = sdk::camera_mgr(pc);
        if (cm) {
            cam_pos = sdk::read<sdk::FVector>(cm + off::APCM_CameraCachePrivate
                                                 + off::FMinimalViewInfo_Location);
            cam_rot = sdk::read<sdk::FRotator>(cm + off::APCM_CameraCachePrivate
                                                 + off::FMinimalViewInfo_Rotation);
            cam_fov = sdk::read<float>(cm + off::APCM_CameraCachePrivate
                                                 + off::FMinimalViewInfo_FOV);
            if (cam_fov < 30 || cam_fov > 170) cam_fov = 103.0f;
        }

        // local team
        if (local_pawn) {
            uintptr_t lps = sdk::read<uintptr_t>(local_pawn + off::APawn_PlayerState);
            if (lps) local_team = sdk::read<int32_t>(lps + off::APS_TeamID);
        }

        // actors
        uintptr_t world = sdk::gworld();
        uintptr_t level = sdk::persistent_lvl(world);
        if (!level) return;

        auto actors = sdk::read<sdk::TArray<uintptr_t>>(level + off::ULevel_Actors);
        int cap = actors.count < 4096 ? actors.count : 4096;

        for (int i = 0; i < cap; ++i) {
            uintptr_t a = actors.at(i);
            if (!a || a == local_pawn) continue;

            uintptr_t ps = sdk::read<uintptr_t>(a + off::APawn_PlayerState);
            if (!ps) continue;

            // ShooterPS health band — cheap filter for "is a player pawn"
            int32_t hp     = sdk::read<int32_t>(ps + off::ShooterPS_Health);
            int32_t hp_max = sdk::read<int32_t>(ps + off::ShooterPS_MaxHealth);
            if (hp <= 0 || hp_max <= 0 || hp_max > 500) continue;

            int32_t team = sdk::read<int32_t>(ps + off::APS_TeamID);
            if (state::cfg.esp_team_check && team == local_team) continue;

            Snapshot s;
            s.pawn   = a;
            s.ps     = ps;
            s.team   = team;
            s.hp     = hp;
            s.hp_max = hp_max;
            s.downed = sdk::read<uint8_t>(ps + off::ShooterPS_bIsDowned) & 1;
            s.head   = sdk::bone_world(a, off::bone::Head);
            s.pelvis = sdk::bone_world(a, off::bone::Pelvis);
            s.feet   = sdk::bone_world(a, off::bone::FootL);

            float dist = (s.pelvis - cam_pos).size();
            if (dist > state::cfg.esp_max_dist) continue;

            frame.push_back(s);
        }
    }
}
