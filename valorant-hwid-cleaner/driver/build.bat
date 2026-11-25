@echo off
rem build.bat — compiles the spoofer driver using EWDK. edit EWDK_ROOT if yours differs.
setlocal
if "%EWDK_ROOT%"=="" set EWDK_ROOT=C:\EWDK
if not exist "%EWDK_ROOT%\BuildEnv\SetupBuildEnv.cmd" (
  echo Set EWDK_ROOT to the EWDK extraction folder [currently %EWDK_ROOT%]
  exit /b 1
)
call "%EWDK_ROOT%\BuildEnv\SetupBuildEnv.cmd" x64
cl /nologo /c /EHs-c- /GR- /Gm- /Gz /W3 /Zi /O2 /D_AMD64_ /DNDIS_WDM /kernel /GS- ^
   /I"%EWDK_ROOT%\Program Files\Windows Kits\10\Include\10.0.22621.0\km" ^
   /I"%EWDK_ROOT%\Program Files\Windows Kits\10\Include\10.0.22621.0\km\crt" ^
   /I"%EWDK_ROOT%\Program Files\Windows Kits\10\Include\10.0.22621.0\shared" ^
   spoofer.cpp hooks.cpp
link /nologo /SUBSYSTEM:NATIVE /DRIVER /ENTRY:DriverEntry /INCREMENTAL:NO ^
     /LIBPATH:"%EWDK_ROOT%\Program Files\Windows Kits\10\Lib\10.0.22621.0\km\x64" ^
     /OUT:spoofer.sys spoofer.obj hooks.obj ntoskrnl.lib hal.lib wdmsec.lib bufferoverflowfastfailk.lib
endlocal
