// in-game menu (ImGui). opened with INSERT. kept tiny to not look like a
// generic cheat UI during streamer screen-captures.

#pragma once
#include "imgui/imgui.h"
#include "state.h"

namespace menu {

    inline void toggle_key_if_pressed() {
        static bool prev = false;
        bool down = (GetAsyncKeyState(state::cfg.menu_key) & 0x8000) != 0;
        if (down && !prev) state::cfg.menu_open = !state::cfg.menu_open;
        prev = down;
    }

    inline void render() {
        toggle_key_if_pressed();
        if (!state::cfg.menu_open) return;

        ImGui::SetNextWindowSize({360, 520}, ImGuiCond_Once);
        if (!ImGui::Begin("veltrix", nullptr,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::End();
            return;
        }

        bool v;
        v = state::cfg.enabled;      ImGui::Checkbox("master",     &v); state::cfg.enabled      = v;

        if (ImGui::CollapsingHeader("esp", ImGuiTreeNodeFlags_DefaultOpen)) {
            v = state::cfg.esp_box;       ImGui::Checkbox("box",        &v); state::cfg.esp_box       = v;
            v = state::cfg.esp_skeleton;  ImGui::Checkbox("skeleton",   &v); state::cfg.esp_skeleton  = v;
            v = state::cfg.esp_health;    ImGui::Checkbox("health",     &v); state::cfg.esp_health    = v;
            v = state::cfg.esp_distance;  ImGui::Checkbox("distance",   &v); state::cfg.esp_distance  = v;
            v = state::cfg.esp_team_check;ImGui::Checkbox("team check", &v); state::cfg.esp_team_check= v;
            float d = state::cfg.esp_max_dist;
            ImGui::SliderFloat("max dist (uu)", &d, 500.0f, 20000.0f, "%.0f");
            state::cfg.esp_max_dist = d;
        }

        if (ImGui::CollapsingHeader("aim", ImGuiTreeNodeFlags_DefaultOpen)) {
            v = state::cfg.aim_enabled;   ImGui::Checkbox("enable",     &v); state::cfg.aim_enabled   = v;
            v = state::cfg.aim_visonly;   ImGui::Checkbox("vis-only",   &v); state::cfg.aim_visonly   = v;
            v = state::cfg.aim_ads_only;  ImGui::Checkbox("ads-only",   &v); state::cfg.aim_ads_only  = v;
            v = state::cfg.aim_humanize;  ImGui::Checkbox("humanize",   &v); state::cfg.aim_humanize  = v;

            const char* bones[] = { "head","neck","chest","pelvis" };
            int b = 0;
            if (state::cfg.aim_bone == off::bone::Neck)   b = 1;
            else if (state::cfg.aim_bone == off::bone::Chest)  b = 2;
            else if (state::cfg.aim_bone == off::bone::Pelvis) b = 3;
            ImGui::Combo("bone", &b, bones, 4);
            state::cfg.aim_bone =
                b == 0 ? off::bone::Head :
                b == 1 ? off::bone::Neck :
                b == 2 ? off::bone::Chest : off::bone::Pelvis;

            float f = state::cfg.aim_fov;    ImGui::SliderFloat("fov",    &f, 1.0f, 30.0f, "%.1f");  state::cfg.aim_fov    = f;
            float s = state::cfg.aim_smooth; ImGui::SliderFloat("smooth", &s, 1.0f, 15.0f, "%.1f");  state::cfg.aim_smooth = s;
        }

        if (ImGui::CollapsingHeader("trigger")) {
            v = state::cfg.trig_enabled;  ImGui::Checkbox("enable",   &v); state::cfg.trig_enabled  = v;
            v = state::cfg.trig_ads_only; ImGui::Checkbox("ads-only", &v); state::cfg.trig_ads_only = v;
            int d = state::cfg.trig_delay_ms;
            ImGui::SliderInt("delay (ms)", &d, 0, 300);
            state::cfg.trig_delay_ms = d;
        }

        if (ImGui::CollapsingHeader("misc")) {
            v = state::cfg.no_recoil; ImGui::Checkbox("no recoil", &v); state::cfg.no_recoil = v;
            ImGui::TextDisabled("INSERT toggles menu · END unloads");
        }

        ImGui::End();
    }
}
