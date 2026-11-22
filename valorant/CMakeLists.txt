cmake_minimum_required(VERSION 3.20)
project(veltrix-val LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ImGui expected at third_party/imgui (docking branch), DX11 + Win32 backends.
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)

add_library(veltrix SHARED
    src/entry.cpp
    src/hooks.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
)

target_include_directories(veltrix PRIVATE
    src
    ${IMGUI_DIR}
)

# relative include for "imgui/imgui.h" paths used in src/
target_include_directories(veltrix PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

if (MSVC)
    target_compile_options(veltrix PRIVATE
        /W4 /std:c++20 /GS- /GR- /EHa
        /O2 /Oi /GL /Gy
        $<$<CONFIG:Release>:/MT>
    )
    target_link_options(veltrix PRIVATE
        /ENTRY:DllMain
        /DEBUG:NONE
        /INCREMENTAL:NO
        /LTCG
        /NODEFAULTLIB:libcmt.lib
        /SAFESEH:NO
    )
    target_compile_definitions(veltrix PRIVATE
        _CRT_SECURE_NO_WARNINGS
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
endif()

target_link_libraries(veltrix PRIVATE
    d3d11
    dxgi
    user32
    gdi32
    ntdll
)
