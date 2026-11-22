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
        src/shaders.cpp
        src/shaders.hpp
        src/input.cpp
        src/input.hpp
        src/asset_manager.cpp
        src/asset_manager.hpp
        src/camera.cpp
        src/camera.hpp
        src/types.hpp
)

target_compile_definitions(SDL3GPU_test PRIVATE
        VULKAN_HPP_NO_CONSTRUCTORS
)

target_compile_options(SDL3GPU_test PRIVATE -g -Og)

add_subdirectory(deps)
