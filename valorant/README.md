# veltrix-val — internal for valorant

target: `VALORANT-Win64-Shipping.exe` (UE4.27-rebased, custom shipping config)
anticheat: Vanguard (vgk.sys, vgc.exe, vgm.dll in protected processes; ring-0 integrity, periodic user-mode module walk, VAD/PFN scans, PT walks on suspect pages, HWID + motherboard fuse binding)
build env: MSVC 19.4x, /std:c++20, /GS- /GR- /EHa, x64 Release, no RTC, no SDL check
artifact: raw PIC payload (no DllMain, no PE loader call) — see `tools/bin2sc.py`

## loading

do not `LoadLibrary`. do not `MmLoadSystemImage`. Vanguard's ring-0 image callback + VAD-type verification kills both.

the only survivable primitive is **manual map into a signed-process VAD backed by `SEC_IMAGE`** or a code-cave jmp from an already-loaded signed module. pipeline:

1. BYOVD loader (e.g. `iqvw64e.sys`, `RTCore64.sys`, `gdrv.sys` — rotate weekly, Vanguard revokes) → arbitrary phys read/write
2. carve `MiAllocateVirtualMemory` → commit RWX inside the game's working set, tagged as `Image`
3. relocate the shellcode (`veltrix.bin`), fix imports via ntdll + user32 hashes (no IAT, no string table)
4. patch `KiUserApcDispatcher` thread → queue APC pointing at our entry
5. first instruction of our entry wipes PE header, zeros section table, re-pages the region as `MEM_PRIVATE` without the `Image` bit after init

Vanguard sweeps module list every ~400ms. anything with `DOS` signature in a VAD labelled `Private` and `+X` is flagged within one cycle → wipe header before first `Present` lands.

## offsets

every offset in `src/offsets.h` is a per-build derivation. Riot ships a new build 2–3× per act; sigs rotate, GWorld moves, bone enums get scrambled. derivation method per symbol is annotated inline (`// DERIVE:`). dump pipeline:

- IDA / BinaryNinja on fresh `VALORANT-Win64-Shipping.exe`
- `uc-dumper` (unprotected UE SDK dumper) run via mapped stub, output `SDK.json`
- feed JSON through `tools/sdk_to_offsets.py` → regenerates `offsets.h`

## rendering

no d3d11 inline hook — Vanguard hashes the first 32 bytes of `IDXGISwapChain::Present` every ~800ms. we use **VEH + PAGE_GUARD** on the Present prologue:

- resolve game's swap chain via dummy window `D3D11CreateDeviceAndSwapChain`
- read `swapchain->lpVtbl[8]` (Present) — this is a game code page
- `VirtualProtect` +GUARD on that page
- VEH handler catches `STATUS_GUARD_PAGE_VIOLATION`, single-steps, re-arms
- on hit → ImGui frame built from our ESP state, then original flow continues

no bytes are modified. nothing to hash.

## usable scope

ESP (boxes, skeleton, health, distance, nametag) · aimbot (FOV, smooth, bone select, visibility via `LineTraceSingleByChannel`) · triggerbot (ADS gate + crosshair trace) · recoil (write `PC->RecoilVector` to zero each tick)

no silent aim (server auth kills it instantly). no bhop (Val has no air-strafing). no wallbang predictor (penetration is client-side rolled, not predictable without server seed).

## kill-switch

`End` key wipes RWX region, drops VEH handler, restores guard pages, exits thread. safer than letting the loader bail.
