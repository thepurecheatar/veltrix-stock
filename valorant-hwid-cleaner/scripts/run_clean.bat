@echo off
rem run_clean.bat — full pass. invoke from an elevated prompt.
setlocal
set TOOLS=%~dp0..\tools
if not exist "%TOOLS%\AMIDEWINx64.EXE" if not exist "%TOOLS%\H2OSDE-Wx64-RW.exe" (
    echo WARNING: no vendor SMBIOS tool in %TOOLS% — firmware strings will NOT move.
    echo drop AMIDEWINx64.EXE (AMI boards) or H2OSDE-Wx64-RW.exe (Insyde laptops) there.
    echo continuing anyway...
)
"%~dp0..\build\Release\jc_hwid_cleaner.exe" %*
