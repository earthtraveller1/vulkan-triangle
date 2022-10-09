cmake_minimum_required(VERSION 3.13)

project(VulkanTriangle)

add_subdirectory(deps/fmt)

add_executable(vulkan-triangle src/main.cpp)

target_include_directories(vulkan-triangle PRIVATE deps/fmt/include)
target_link_libraries(vulkan-triangle fmt)