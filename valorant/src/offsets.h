// C++20 / MSVC / Windows 11 / target: VALORANT-Win64-Shipping.exe
// per-build derivation table. all values rotate every Riot ship (~2-3x/act).
// derive fresh with tools/sdk_to_offsets.py against the current binary.

#pragma once
#include <cstdint>

namespace off {

    // engine singletons
    // DERIVE: xref "SeamlessTravel FAILED" wstring, nearest mov rax,[rip+disp] -> GWorld
    constexpr uintptr_t GWorld            = 0x0;   // FILL: pattern "48 89 05 ? ? ? ? 48 8B 4B 78 48 8D 05"
    // DERIVE: GNames ptr lives next to GNamePool in shipping. sig pattern below.
    constexpr uintptr_t GNames            = 0x0;   // FILL: pattern "48 8D 0D ? ? ? ? E8 ? ? ? ? C6 05"

    // UWorld
    constexpr uint32_t UWorld_PersistentLevel      = 0x38;   // FILL: DERIVE struct dump
    constexpr uint32_t UWorld_OwningGameInstance   = 0x1C0;  // FILL
    constexpr uint32_t UWorld_GameState            = 0x140;  // FILL

    // UGameInstance
    constexpr uint32_t UGameInstance_LocalPlayers  = 0x38;   // FILL (TArray<ULocalPlayer*>)

    // ULocalPlayer
    constexpr uint32_t ULocalPlayer_PlayerController = 0x30; // FILL

    // APlayerController
    constexpr uint32_t APlayerController_AcknowledgedPawn   = 0x338; // FILL
    constexpr uint32_t APlayerController_PlayerCameraManager= 0x348; // FILL
    constexpr uint32_t APlayerController_ControlRotation    = 0x2A8; // FILL (FRotator pitch/yaw/roll)
    constexpr uint32_t APlayerController_InputYawScale      = 0x0;   // FILL

    // APlayerCameraManager
    constexpr uint32_t APCM_CameraCachePrivate     = 0x1F80;         // FILL (FMinimalViewInfo at +0x10)
    constexpr uint32_t FMinimalViewInfo_Location   = 0x10;
    constexpr uint32_t FMinimalViewInfo_Rotation   = 0x1C;
    constexpr uint32_t FMinimalViewInfo_FOV        = 0x28;

    // ULevel
    constexpr uint32_t ULevel_Actors               = 0x98;   // FILL (TArray<AActor*>)

    // AActor
    constexpr uint32_t AActor_RootComponent        = 0x198;  // FILL
    constexpr uint32_t AActor_bHidden              = 0x92;   // FILL (bit 0)

    // APawn / ACharacter
    constexpr uint32_t APawn_PlayerState           = 0x2A0;  // FILL
    constexpr uint32_t ACharacter_Mesh             = 0x330;  // FILL (USkeletalMeshComponent*)

    // APlayerState  (Riot subclass: ShooterPlayerState)
    constexpr uint32_t APS_TeamID                  = 0x38C;  // FILL
    constexpr uint32_t APS_PlayerName_FString      = 0x340;  // FILL (FString, {ptr,count})
    constexpr uint32_t ShooterPS_Health            = 0x690;  // FILL
    constexpr uint32_t ShooterPS_MaxHealth         = 0x694;  // FILL
    constexpr uint32_t ShooterPS_bIsDowned         = 0x6A1;  // FILL

    // USceneComponent
    constexpr uint32_t USceneComp_ComponentToWorld = 0x1C0;  // FILL (FTransform, 48B: Rot q,Trans v3,Scale v3)

    // USkeletalMeshComponent — bone matrices live in ComponentSpaceTransformsArray
    constexpr uint32_t USkelMesh_BoneArray         = 0x5C0;  // FILL (TArray<FTransform>, idx by bone id)
    constexpr uint32_t USkelMesh_CachedBoneTx      = 0x590;  // FILL (alt: cached for minimal replication)

    // weapon / recoil
    constexpr uint32_t PC_CurrentWeapon            = 0x8E0;  // FILL (AShooterWeapon*)
    constexpr uint32_t Wpn_RecoilVector            = 0xA40;  // FILL (FVector2D cumulative kick)
    constexpr uint32_t Wpn_bIsScoping              = 0xB18;  // FILL (bool)

    // bone ids — Riot scrambles these per-build. dump via SkeletalMesh->RefSkeleton names.
    // DERIVE: iterate RefSkeleton.RawRefBoneInfo, match FName -> index.
    namespace bone {
        constexpr int32_t Head      = 8;   // FILL
        constexpr int32_t Neck      = 7;   // FILL
        constexpr int32_t Chest     = 6;   // FILL
        constexpr int32_t Pelvis    = 2;   // FILL
        constexpr int32_t UpperArmL = 10;  // FILL
        constexpr int32_t LowerArmL = 11;  // FILL
        constexpr int32_t HandL     = 12;  // FILL
        constexpr int32_t UpperArmR = 20;  // FILL
        constexpr int32_t LowerArmR = 21;  // FILL
        constexpr int32_t HandR     = 22;  // FILL
        constexpr int32_t ThighL    = 60;  // FILL
        constexpr int32_t CalfL     = 61;  // FILL
        constexpr int32_t FootL     = 62;  // FILL
        constexpr int32_t ThighR    = 65;  // FILL
        constexpr int32_t CalfR     = 66;  // FILL
        constexpr int32_t FootR     = 67;  // FILL
    }
}
