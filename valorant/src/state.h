// shared config + runtime state across hook / esp / aim / trigger.
// written from the ImGui menu on the render thread, read everywhere.

#pragma once
#include <atomic>
#include <cstdint>

namespace state {

    struct Config {
        // master
        std::atomic<bool>  enabled       { true };

        // esp
        std::atomic<bool>  esp_box       { true };
        std::atomic<bool>  esp_skeleton  { true };
        std::atomic<bool>  esp_health    { true };
        std::atomic<bool>  esp_name      { false };
        std::atomic<bool>  esp_distance  { true };
        std::atomic<float> esp_max_dist  { 12000.0f };   // UE units (~120m)
        std::atomic<bool>  esp_team_check{ true };

        // aim
        std::atomic<bool>  aim_enabled   { true };
        std::atomic<bool>  aim_visonly   { true };
        std::atomic<int>   aim_bone      { 8 };          // head id — override from menu
        std::atomic<float> aim_fov       { 6.0f };       // degrees
        std::atomic<float> aim_smooth    { 4.5f };       // 1 = snap, 10 = butter
        std::atomic<int>   aim_key       { 0x02 };       // VK_RBUTTON
        std::atomic<bool>  aim_ads_only  { false };
        std::atomic<bool>  aim_humanize  { true };

        // trigger
        std::atomic<bool>  trig_enabled  { false };
        std::atomic<int>   trig_key      { 0x06 };       // VK_XBUTTON2
        std::atomic<int>   trig_delay_ms { 30 };
        std::atomic<bool>  trig_ads_only { true };

        // recoil
        std::atomic<bool>  no_recoil     { false };

        // menu
        std::atomic<int>   menu_key      { 0x2D };       // VK_INSERT
        std::atomic<bool>  menu_open     { false };
    };

    inline Config cfg;
}
