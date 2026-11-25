cmake_minimum_required(VERSION 3.31)
project(SDL3GPU_test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(SDL3GPU_test
        src/main.cpp
        src/renderer.cpp
        src/renderer.hpp
        src/input.cpp
        src/input.hpp
        src/asset_manager.cpp
        src/asset_manager.hpp
        src/types.hpp
        src/shader_manager.cpp
        src/shader_manager.hpp
        src/components/entity.cpp
        src/components/entity.hpp
        src/components/component.hpp
        src/components/transform_component.cpp
        src/components/transform_component.hpp
        src/components/camera_component.cpp
        src/components/camera_component.hpp
        src/components/player_entity.cpp
        src/components/player_entity.hpp
        src/components/scene.cpp
        src/components/scene.hpp
        src/components/scene_manager.cpp
        src/components/scene_manager.hpp
        src/components/mesh_component.cpp
        src/components/mesh_component.hpp
)

target_include_directories(SDL3GPU_test PRIVATE
        src/
)

target_compile_definitions(SDL3GPU_test PRIVATE
        VULKAN_HPP_NO_CONSTRUCTORS
)

target_compile_options(SDL3GPU_test PRIVATE -g -Og)

add_subdirectory(deps)
