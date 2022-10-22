cmake_minimum_required(VERSION 3.13)

project(VulkanTriangle)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

if (WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
endif()

add_library(vulkan SHARED IMPORTED)
add_subdirectory(deps)

add_executable(vulkan-triangle src/main.cpp)

target_include_directories(vulkan-triangle PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/fmt/include
    ${CMAKE_SOURCE_DIR}/deps/glfw/include
    ${CMAKE_SOURCE_DIR}/deps/Vulkan-Headers/include
)

target_link_libraries(vulkan-triangle glfw fmt vulkan)