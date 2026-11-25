@echo off
rem build.bat — builds usermode exe using Visual Studio Build Tools / MSVC x64
setlocal
pushd %~dp0..
if not exist build mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
if errorlevel 1 goto :fail
cmake --build . --config Release
if errorlevel 1 goto :fail
echo.
echo built: build\Release\jc_hwid_cleaner.exe
popd
exit /b 0
:fail
echo build failed
popd
exit /b 1
