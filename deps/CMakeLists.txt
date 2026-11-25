# Vulkan | vulkan-sdk-1.4.363
# glm | 1.0.3
# imgui | v1.93.0-docking
# fastgltf | v0.9.0
# SDL | release-3.4.20
# SDL_shadercross | 1ff05bec
# tracy | v0.15.0

find_package(Vulkan)

add_library(imgui STATIC EXCLUDE_FROM_ALL)
target_include_directories(imgui PUBLIC imgui)
target_sources(imgui PRIVATE
        imgui/imgui.h
        imgui/imgui.cpp

        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/backends/imgui_impl_sdl3.cpp
        imgui/backends/imgui_impl_sdlgpu3.cpp
)

add_subdirectory(fastgltf SYSTEM EXCLUDE_FROM_ALL)

set(SDL_SHARED ON)
set(SDL_STATIC OFF)
set(SDL_CCACHE ON)
add_subdirectory(SDL SYSTEM EXCLUDE_FROM_ALL)

add_subdirectory(SDL_shadercross SYSTEM EXCLUDE_FROM_ALL)

set(TRACY_ENABLE OFF)
set(TRACY_ON_DEMAND ON)
add_subdirectory(tracy SYSTEM EXCLUDE_FROM_ALL)

add_subdirectory(glm SYSTEM EXCLUDE_FROM_ALL)
target_compile_definitions(SDL3GPU_test PUBLIC
        GLM_ENABLE_EXPERIMENTAL
        GLM_FORCE_DEPTH_ZERO_TO_ONE
)

target_link_libraries(SDL3GPU_test PRIVATE
        Vulkan::Vulkan
        SDL3::SDL3
        SDL3_shadercross::SDL3_shadercross
        glm::glm
        fastgltf::fastgltf
        Tracy::TracyClient
        imgui
)
