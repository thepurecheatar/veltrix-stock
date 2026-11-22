// world-to-screen. matches UE4 APlayerController::ProjectWorldLocationToScreen
// without crossing into their function (hashed). we rebuild from MinimalViewInfo.

#pragma once
#include "sdk.h"

namespace w2s {

    struct Screen { float x, y; bool on; };

    inline void build_view_matrix(const sdk::FRotator& r, sdk::FVector axes[3]) {
        const float d2r = 3.14159265358979f / 180.0f;
        float cp = std::cos(r.pitch * d2r), sp = std::sin(r.pitch * d2r);
        float cy = std::cos(r.yaw   * d2r), sy = std::sin(r.yaw   * d2r);
        float cr = std::cos(r.roll  * d2r), sr = std::sin(r.roll  * d2r);
        axes[0] = { cp*cy, cp*sy, sp };                                   // X forward
        axes[1] = { sr*sp*cy - cr*sy, sr*sp*sy + cr*cy, -sr*cp };         // Y right
        axes[2] = { -(cr*sp*cy + sr*sy), cy*sr - cr*sp*sy, cr*cp };       // Z up (FRotationMatrix row 3)
    }

    inline Screen project(const sdk::FVector& world,
                          const sdk::FVector& cam_pos,
                          const sdk::FRotator& cam_rot,
                          float fov_deg,
                          float screen_w,
                          float screen_h)
    {
        Screen s{0,0,false};
        sdk::FVector axes[3];
        build_view_matrix(cam_rot, axes);

        sdk::FVector d = world - cam_pos;
        float fwd   = d.dot(axes[0]);
        if (fwd < 1.0f) return s;
        float right = d.dot(axes[1]);
        float up    = d.dot(axes[2]);

        float fov_rad = fov_deg * (3.14159265358979f / 180.0f);
        float fov_scale = screen_w * 0.5f / std::tan(fov_rad * 0.5f);

        s.x = (screen_w * 0.5f) + right * fov_scale / fwd;
        s.y = (screen_h * 0.5f) - up    * fov_scale / fwd;
        s.on = (s.x >= 0 && s.x < screen_w && s.y >= 0 && s.y < screen_h);
        return s;
    }
}
