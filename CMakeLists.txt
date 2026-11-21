cmake_minimum_required(VERSION 3.31)
project(SDL3GPU_test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)

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
)

target_compile_definitions(SDL3GPU_test PRIVATE
        VULKAN_HPP_NO_CONSTRUCTORS
)

find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(SDL3GPU_test PRIVATE SDL3::SDL3)

find_package(SDL3_image)
target_link_libraries(SDL3GPU_test PRIVATE SDL3_image::SDL3_image)

find_package(SDL3_shadercross CONFIG REQUIRED)
target_link_libraries(SDL3GPU_test PRIVATE SDL3_shadercross::SDL3_shadercross)

find_package(glm CONFIG REQUIRED)
target_link_libraries(SDL3GPU_test PRIVATE glm::glm)

find_package(fastgltf CONFIG REQUIRED)
target_link_libraries(SDL3GPU_test PRIVATE fastgltf::fastgltf)

find_package(Tracy CONFIG REQUIRED)
target_link_libraries(SDL3GPU_test PRIVATE Tracy::TracyClient)

find_package(Vulkan)
target_link_libraries(SDL3GPU_test PRIVATE Vulkan::Vulkan)

