// ESP draw. runs inside the Present hook, before original Present.
// uses ImGui background drawlist so we never allocate d3d resources ourselves —
// the game owns the device, we just borrow its command list.

#pragma once
#include <cstdio>
#include "imgui/imgui.h"
#include "entities.h"
#include "w2s.h"
#include "state.h"

namespace esp {

    inline ImU32 hp_color(int hp, int hp_max) {
        float t = (float)hp / (float)(hp_max ? hp_max : 100);
        if (t < 0) t = 0; if (t > 1) t = 1;
        int r = (int)((1.0f - t) * 255);
        int g = (int)(t * 220);
        return IM_COL32(r, g, 40, 255);
    }

    inline void draw_box(ImDrawList* dl, const ImVec2& head, const ImVec2& feet, ImU32 col) {
        float h = feet.y - head.y;
        if (h < 10) return;
        float w = h * 0.45f;
        ImVec2 tl{ head.x - w*0.5f, head.y - h*0.08f };
        ImVec2 br{ head.x + w*0.5f, feet.y };
        // outline + dark halo for contrast
        dl->AddRect({tl.x-1,tl.y-1}, {br.x+1,br.y+1}, IM_COL32(0,0,0,180), 0, 0, 2.0f);
        dl->AddRect(tl, br, col, 0, 0, 1.2f);
    }

    inline void draw_hp(ImDrawList* dl, const ImVec2& head, const ImVec2& feet,
                        int hp, int hp_max) {
        float h = feet.y - head.y;
        if (h < 10) return;
        float w = h * 0.45f;
        float bx = head.x - w*0.5f - 6.0f;
        float by0 = head.y - h*0.08f;
        float by1 = feet.y;
        float t = (float)hp / (float)(hp_max ? hp_max : 100);
        if (t < 0) t = 0; if (t > 1) t = 1;
        dl->AddRectFilled({bx-1, by0-1}, {bx+4, by1+1}, IM_COL32(0,0,0,200));
        dl->AddRectFilled({bx, by1 - (by1-by0)*t}, {bx+3, by1}, hp_color(hp, hp_max));
    }

    inline void draw_skeleton(ImDrawList* dl, uintptr_t pawn,
                              const sdk::FVector& cam, const sdk::FRotator& rot,
                              float fov, float sw, float sh, ImU32 col)
    {
        struct Link { int a, b; };
        static const Link links[] = {
            { off::bone::Head, off::bone::Neck },
            { off::bone::Neck, off::bone::Chest },
            { off::bone::Chest, off::bone::Pelvis },
            { off::bone::Chest, off::bone::UpperArmL }, { off::bone::UpperArmL, off::bone::LowerArmL }, { off::bone::LowerArmL, off::bone::HandL },
            { off::bone::Chest, off::bone::UpperArmR }, { off::bone::UpperArmR, off::bone::LowerArmR }, { off::bone::LowerArmR, off::bone::HandR },
            { off::bone::Pelvis, off::bone::ThighL }, { off::bone::ThighL, off::bone::CalfL }, { off::bone::CalfL, off::bone::FootL },
            { off::bone::Pelvis, off::bone::ThighR }, { off::bone::ThighR, off::bone::CalfR }, { off::bone::CalfR, off::bone::FootR },
        };
        for (auto& l : links) {
            auto pa = sdk::bone_world(pawn, l.a);
            auto pb = sdk::bone_world(pawn, l.b);
            auto sa = w2s::project(pa, cam, rot, fov, sw, sh);
            auto sb = w2s::project(pb, cam, rot, fov, sw, sh);
            if (!sa.on || !sb.on) continue;
            dl->AddLine({sa.x, sa.y}, {sb.x, sb.y}, IM_COL32(0,0,0,180), 2.2f);
            dl->AddLine({sa.x, sa.y}, {sb.x, sb.y}, col, 1.2f);
        }
    }

    inline void render() {
        if (!state::cfg.enabled) return;
        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImGuiIO& io = ImGui::GetIO();
        float sw = io.DisplaySize.x, sh = io.DisplaySize.y;

        for (const auto& e : ents::frame) {
            auto head = w2s::project(e.head,   ents::cam_pos, ents::cam_rot, ents::cam_fov, sw, sh);
            auto feet = w2s::project(e.feet,   ents::cam_pos, ents::cam_rot, ents::cam_fov, sw, sh);
            if (!head.on || !feet.on) continue;

            ImU32 col = e.downed ? IM_COL32(120,120,120,255) : IM_COL32(255, 60, 90, 255);

            if (state::cfg.esp_box)
                draw_box(dl, {head.x, head.y}, {feet.x, feet.y}, col);

            if (state::cfg.esp_health)
                draw_hp(dl, {head.x, head.y}, {feet.x, feet.y}, e.hp, e.hp_max);

            if (state::cfg.esp_skeleton)
                draw_skeleton(dl, e.pawn, ents::cam_pos, ents::cam_rot, ents::cam_fov, sw, sh, col);

            if (state::cfg.esp_distance) {
                float d = (e.pelvis - ents::cam_pos).size() / 100.0f; // UE unit ~1cm
                char buf[32]; _snprintf_s(buf, _TRUNCATE, "%.0fm", d);
                ImVec2 ts = ImGui::CalcTextSize(buf);
                dl->AddText({feet.x - ts.x*0.5f + 1, feet.y + 3}, IM_COL32(0,0,0,220), buf);
                dl->AddText({feet.x - ts.x*0.5f,     feet.y + 2}, IM_COL32(255,255,255,255), buf);
            }
        }
    }
}
