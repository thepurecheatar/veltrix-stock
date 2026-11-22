// UE4 SDK subset for Valorant — only what ESP/aim/trigger touch.
// read-side types; never alloc one of these on our stack.

#pragma once
#include <cstdint>
#include <cmath>
#include "offsets.h"

namespace sdk {

    struct FVector {
        float x, y, z;
        FVector operator-(const FVector& o) const { return {x-o.x, y-o.y, z-o.z}; }
        FVector operator+(const FVector& o) const { return {x+o.x, y+o.y, z+o.z}; }
        float   dot(const FVector& o)       const { return x*o.x + y*o.y + z*o.z; }
        float   size()                      const { return std::sqrt(x*x + y*y + z*z); }
        float   size2d()                    const { return std::sqrt(x*x + y*y); }
    };

    struct FVector2D { float x, y; };

    struct FRotator {
        float pitch, yaw, roll;
        FRotator normalize() const {
            auto wrap = [](float v) {
                v = std::fmod(v + 180.0f, 360.0f);
                if (v < 0) v += 360.0f;
                return v - 180.0f;
            };
            return { wrap(pitch), wrap(yaw), wrap(roll) };
        }
    };

    struct FQuat { float x, y, z, w; };

    struct FTransform {        // UE4 aligned, 48B
        FQuat    rot;
        FVector  trans; float _pad0;
        FVector  scale; float _pad1;
    };

    template<typename T>
    struct TArray {
        T*       data;
        int32_t  count;
        int32_t  max;
        T at(int i) const {
            T out{};
            if (!data || i < 0 || i >= count) return out;
            __try { out = data[i]; } __except(1) { }
            return out;
        }
    };

    struct FString {
        wchar_t* data;
        int32_t  count;
        int32_t  max;
    };

    // helpers — safe read; caller passes a base
    template<typename T>
    inline T read(uintptr_t addr) {
        T v{};
        if (!addr) return v;
        __try { v = *reinterpret_cast<T*>(addr); } __except(1) { }
        return v;
    }
    template<typename T>
    inline bool write(uintptr_t addr, const T& v) {
        if (!addr) return false;
        __try { *reinterpret_cast<T*>(addr) = v; return true; } __except(1) { return false; }
    }

    // accessors
    uintptr_t game_base();                                          // defined in entry.cpp
    inline uintptr_t gworld()        { return read<uintptr_t>(game_base() + off::GWorld); }
    inline uintptr_t persistent_lvl(uintptr_t w)  { return read<uintptr_t>(w + off::UWorld_PersistentLevel); }
    inline uintptr_t game_instance(uintptr_t w)   { return read<uintptr_t>(w + off::UWorld_OwningGameInstance); }

    inline uintptr_t local_player(uintptr_t gi) {
        TArray<uintptr_t> arr = read<TArray<uintptr_t>>(gi + off::UGameInstance_LocalPlayers);
        return arr.at(0);
    }
    inline uintptr_t player_controller(uintptr_t lp) {
        return read<uintptr_t>(lp + off::ULocalPlayer_PlayerController);
    }
    inline uintptr_t local_pawn(uintptr_t pc) {
        return read<uintptr_t>(pc + off::APlayerController_AcknowledgedPawn);
    }
    inline uintptr_t camera_mgr(uintptr_t pc) {
        return read<uintptr_t>(pc + off::APlayerController_PlayerCameraManager);
    }

    // actor bone world position: ComponentToWorld.Transform * BoneArray[id].Trans
    // we keep it to translation only — scale is 1 and rot is unused for a point.
    inline FVector bone_world(uintptr_t pawn, int32_t bone_id) {
        FVector out{};
        uintptr_t root = read<uintptr_t>(pawn + off::AActor_RootComponent);
        uintptr_t mesh = read<uintptr_t>(pawn + off::ACharacter_Mesh);
        if (!mesh || !root) return out;

        FTransform c2w = read<FTransform>(mesh + off::USceneComp_ComponentToWorld);
        TArray<FTransform> bones = read<TArray<FTransform>>(mesh + off::USkelMesh_BoneArray);
        FTransform b = bones.at(bone_id);
        if (bones.count == 0) b = read<TArray<FTransform>>(mesh + off::USkelMesh_CachedBoneTx).at(bone_id);

        // quaternion rotate(bone.trans) + c2w.trans
        float qx=c2w.rot.x, qy=c2w.rot.y, qz=c2w.rot.z, qw=c2w.rot.w;
        float vx=b.trans.x, vy=b.trans.y, vz=b.trans.z;
        float tx = 2.0f * (qy*vz - qz*vy);
        float ty = 2.0f * (qz*vx - qx*vz);
        float tz = 2.0f * (qx*vy - qy*vx);
        out.x = vx + qw*tx + (qy*tz - qz*ty) + c2w.trans.x;
        out.y = vy + qw*ty + (qz*tx - qx*tz) + c2w.trans.y;
        out.z = vz + qw*tz + (qx*ty - qy*tx) + c2w.trans.z;
        return out;
    }
}
