cmake_minimum_required(VERSION 3.16)
project(Flux)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FETCHCONTENT_DECLARE(GLFW GIT_REPOSITORY https://github.com/glfw/glfw.git)
FETCHCONTENT_MAKEAVAILABLE(GLFW)

find_package(Vulkan REQUIRED)

add_executable(Flux
    src/swapchain_images.cpp
    src/swapchain.cpp
    src/logical_device.cpp
    src/physical_device.cpp
    src/vulkan_surface.cpp
    src/vulkan_instance.cpp
    src/window.cpp
    src/main.cpp
)

target_include_directories(Flux PUBLIC include)

target_link_libraries(Flux glfw Vulkan::Vulkan)

