cmake_minimum_required(VERSION 3.20)
project(valorant_hwid_cleaner CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "Windows only")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "x64 build only — Vanguard is x64")
endif()

# static CRT, no CRT leaks in the dropped binary
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

add_compile_options(/W3 /permissive- /Zc:__cplusplus /Zi /EHsc /GS /guard:cf)
add_compile_definitions(
    _WIN32_WINNT=0x0A00 WINVER=0x0A00 NTDDI_VERSION=0x0A000000
    _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN NOMINMAX
    UNICODE _UNICODE
)

file(GLOB SRC
    usermode/src/*.cpp
)
add_executable(jc_hwid_cleaner ${SRC})
target_include_directories(jc_hwid_cleaner PRIVATE usermode/include)
target_link_libraries(jc_hwid_cleaner PRIVATE
    Setupapi Cfgmgr32 Advapi32 Iphlpapi Tbs Wintrust Crypt32 Bcrypt Ntdll Ws2_32
)
# require admin
set_target_properties(jc_hwid_cleaner PROPERTIES LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:CONSOLE")
