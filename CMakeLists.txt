cmake_minimum_required(VERSION 3.13)

project(VulkanTriangle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(Vulkan)

add_subdirectory(deps/fmt)
add_subdirectory(deps/glfw)

add_executable(vulkan-triangle src/main.cpp)

target_include_directories(vulkan-triangle PRIVATE deps/fmt/include deps/glfw/include ${VULKAN_INCLUDE_DIR})
target_link_libraries(vulkan-triangle fmt glfw Vulkan::Vulkan)