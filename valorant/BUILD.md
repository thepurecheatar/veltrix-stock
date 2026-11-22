# build

## prereqs

- MSVC 19.4x with Win11 SDK 10.0.26100+
- CMake ≥ 3.20, Ninja
- ImGui (docking branch) at `valorant/third_party/imgui`:
  ```
  git clone -b docking https://github.com/ocornut/imgui valorant/third_party/imgui
  ```
- Python 3.11+ for `tools/`

## compile

```powershell
cd valorant
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

output: `build/veltrix.dll`

## produce raw payload

```powershell
python tools\bin2sc.py build\veltrix.dll veltrix.bin
```

`veltrix.bin` layout:

```
[0..16)   header: entry_rva | image_size | iat_tbl_off | iat_count
[16..N)   mapped image (sections unfolded, relocs applied against base 0)
```

## derive offsets per Riot build

1. close the game
2. run SDK dumper against `VALORANT-Win64-Shipping.exe` (any UE4.27-compatible dumper; feed `global_tables.h` from the dumper into our hashed-name resolver)
3. dumper emits `sdk.json` with the key set listed in `src/offsets.h` (every `// FILL:` line maps to one JSON key)
4. patch our table:
   ```
   python tools\sdk_to_offsets.py sdk.json
   ```
5. rebuild → re-pack

## load

the DLL will *not* survive LoadLibrary. use a kernel mapper:

- userland half: handshake to the mapper driver via its device object
- driver half (BYOVD): map `veltrix.bin` into the game process VAD at a region carved from `MiAllocateVirtualMemory` with `MEM_COMMIT|MEM_PRIVATE|PAGE_EXECUTE_READWRITE`
- queue a user APC on the main game thread pointing to `header.entry_rva + payload_base`
- free the payload region on `End` key (our thread exits, mapper poll clears the VAD)

## known-good test loop

- spin up a custom offline match with bots (Practice → Open Range)
- verify ESP + distance before touching aim
- the moment the menu shows, press `End` — confirms the kill switch works
- only then attach to a live match on a fresh account; never on your main
