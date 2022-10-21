cmake_minimum_required(VERSION 3.13)

project(VulkanTriangle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(vulkan-triangle src/main.cpp)