cmake_minimum_required(VERSION 3.31)
project(SDL3GPU_test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_CXX_STANDARD 23)

add_executable(SDL3GPU_test
        main.cpp
        renderer.cpp
        renderer.hpp
        shaders.cpp
        shaders.hpp
        input.cpp
        input.hpp
        asset_manager.cpp
        asset_manager.hpp
        renderer_nvrhi.cpp
        renderer_nvrhi.hpp
)

target_compile_definitions(SDL3GPU_test PRIVATE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=0
        VK_NO_PROTOTYPES
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

find_package(volk CONFIG REQUIRED)
target_link_libraries(SDL3GPU_test PRIVATE Vulkan::volk)

find_package(nvrhi CONFIG REQUIRED)
set(NVRHI_WITH_VULKAN ON)
target_link_libraries(SDL3GPU_test PUBLIC nvrhi nvrhi_vk)

