# jailcrash — valorant hardban hwid cleaner

sweep every surface Vanguard fingerprints: SMBIOS, disk serials, volume IDs, MACs, registry GUIDs, TPM EK, GPU + EDID, then uninstall Vanguard and block its reinstall until BIOS is confirmed clean.

built for Windows 11 x64, MSVC C++17.

## layout

```
valorant-hwid-cleaner/
  usermode/                    — admin console app (jc_hwid_cleaner.exe)
    include/                   — one header per module
    src/                       — matching .cpp + main.cpp orchestrator
  driver/                      — optional kernel spoofer (BYOVD manual map)
    spoofer.cpp hooks.cpp      — storage-stack IOCTL filter + hook scaffolding
    build.bat                  — EWDK build
  tools/                       — DROP THESE YOURSELF (not shipped):
      AMIDEWINx64.EXE          — AMI BIOS SMBIOS editor (desktops, ASUS / MSI / Gigabyte / ASRock)
      H2OSDE-Wx64-RW.exe       — Insyde H2O SMBIOS editor (Dell / HP / Lenovo laptops)
      iqvw64e.sys              — Intel Network Adapter Diagnostic (BYOVD gadget)
      RTCore64.sys             — MSI Afterburner (second-stage PTE leak)
      spoofer.sys              — this project's driver, built via driver/build.bat
  scripts/
    build.bat run_clean.bat
  CMakeLists.txt
```

## surfaces moved

| surface                         | mechanism                                                                                | needs reboot |
|---------------------------------|------------------------------------------------------------------------------------------|--------------|
| SMBIOS type 0/1/2/3             | `GetSystemFirmwareTable` readback + vendor tool (`AMIDEWIN` / `H2OSDE`) write            | yes          |
| disk serials (SATA/NVMe)        | driver IOCTL filter on `IOCTL_STORAGE_QUERY_PROPERTY` (kernel spoofer)                   | no (driver)  |
| volume IDs (NTFS/FAT32/exFAT)   | raw VBR patch at 0x48 / 0x43 / 0x64, `FSCTL_DISMOUNT_VOLUME`                             | system vol: yes |
| MAC addresses                   | `HKLM...\{4D36E972...}\NNNN\NetworkAddress` + `DI_PROPCHANGE` disable/enable             | no           |
| MachineGuid                     | `HKLM\SOFTWARE\Microsoft\Cryptography\MachineGuid`                                       | no           |
| SQMClient MachineId             | `HKLM\SOFTWARE\Microsoft\SQMClient\MachineId`                                            | no           |
| WindowsUpdate SusClientId       | `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\WindowsUpdate\SusClientId`               | no           |
| HwProfileGuid                   | all `ControlSetXXX\Control\IDConfigDB\Hardware Profiles\0001`                             | no           |
| Windows install PingID          | `...\WindowsUpdate\Auto Update\PingID`                                                   | no           |
| TPM EK pub                      | `Tbsi_Physical_Presence_Command` op 14 (clear)                                           | yes (consent)|
| GPU UUID / NVIDIA GPUID         | `HKLM\SYSTEM\...\Class\{4D36E968...}\NNNN` string values                                   | no           |
| monitor EDID                    | `HKLM\SYSTEM\...\Enum\DISPLAY\*\*\Device Parameters\EDID`                                 | no           |
| Vanguard install                | stop `vgc`+`vgk`, delete services, wipe `Program Files\Riot Vanguard` + registry + prefetch | yes      |

## usage

```
# 1. build
scripts\build.bat

# 2. drop vendor SMBIOS tool in tools/ (see table above)

# 3. (optional, kernel path) build the driver
cd driver && build.bat && cd ..

# 4. elevated run — full pass
scripts\run_clean.bat
#   --dry-run          only capture snapshot
#   --validate-only    read everything, don't change
#   --keep-vanguard    don't touch Vanguard install
#   --with-driver      load BYOVD + manual-map spoofer.sys
```

output:
- `spoofer.log`       — full trace
- `spoof_report.json` — per-surface moved/not-moved + notes

## reboot discipline

1. first run goes to completion, writes BIOS, requests TPM clear at next boot.
2. reboot. at POST, if TPM asks to clear, press F12 / Y to confirm.
3. before Windows logs in, in BIOS disable "SMBIOS Firmware Lock" if the vendor tool said it was locked.
4. boot clean. first thing: run `scripts\run_clean.bat --validate-only`. all lines should say `moved: yes` or a nonzero count. system volume serial only flips after this boot — that's expected.
5. make a fresh Windows account (local, new name) if Riot has your old machine-name linked.
6. install Valorant from a fresh Riot Client. Vanguard reinstalls itself.

## BYOVD note

the kernel spoofer is gated behind two-stage BYOVD: `iqvw64e.sys` (Intel NAL, phys-mem map primitive) paired with `RTCore64.sys` (MSI, PTE leak). set env var `BYOVD_DO_MAP=1` to enable actual manual map. without that env var the loader stops before patching kernel memory — intentional, so a stray run doesn't BSOD.

if iqvw64e is on Microsoft's recommended driver blocklist on your Windows 11 build, substitute with `DBUtil_2_3.sys` (Dell) — same primitive shape, different IOCTL table.

## when it doesn't stick

| symptom                                | fix                                                                               |
|----------------------------------------|-----------------------------------------------------------------------------------|
| smbios didn't move                     | vendor tool missing, or BIOS SMBIOS lock enabled — disable in firmware setup      |
| mac reverts on boot                    | NIC has firmware-locked MAC — use `--with-driver` so kernel hook lies at IOCTL    |
| system volume serial unchanged         | expected, flips on next boot — re-run after reboot or boot into WinPE and patch   |
| EDID writes denied                     | run as TrustedInstaller: `psexec -s -i cmd.exe`, then run cleaner from there      |
| Vanguard reinstalls from Riot Client   | `block_reinstall` adds `icacls /deny *S-1-5-18` on install dir; stays until `unblock` |

## kernel hook notes

`driver/hooks.cpp` installs a filter above every `\Device\HarddiskN\DR0`. on `IOCTL_STORAGE_QUERY_PROPERTY` completion, it rewrites `STORAGE_DEVICE_DESCRIPTOR.SerialNumberOffset` in-place with the lie table that usermode handed it via `IOCTL_JC_SET_DISK_LIE`. no pool allocation, no growth — fits in the existing buffer.

the NDIS and SMBIOS-firmware-table hooks live under `JC_DO_FW_HOOK` build gates because they need per-build inline trampolines on `ndis!NdisReadNetworkAddress` and `nt!ExpGetSystemFirmwareTable`. offsets differ per KB — compute them from the running ntoskrnl at load time and bake into a tiny trampoline. that step is intentionally per-build.
