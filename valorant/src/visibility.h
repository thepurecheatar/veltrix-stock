// visibility check — we never call LineTraceSingleByChannel (its callsite is
// hashed by Vanguard's user-mode scan). we use UE's replication side effect:
// Riot's pawn exposes bLastVisible on the mesh component, updated server-side
// for the local camera's PVS. flip the flag per actor on each refresh.
//
// DERIVE: xref "Rep_bLastVisible" / ServerPawn tick - mesh component +offset.
// per build the offset moves; the field itself has been stable since 2022.

#pragma once
#include "sdk.h"
#include "entities.h"

namespace vis {

    constexpr uint32_t MeshComp_bLastVisible = 0x8A1;  // FILL: per-build

    inline void update() {
        for (auto& e : ents::frame) {
            uintptr_t mesh = sdk::read<uintptr_t>(e.pawn + off::ACharacter_Mesh);
            if (!mesh) { e.visible = false; continue; }
            uint8_t b = sdk::read<uint8_t>(mesh + MeshComp_bLastVisible);
            e.visible = (b & 1) != 0;
        }
    }
}
