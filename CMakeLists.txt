cmake_minimum_required(VERSION 3.13)

project(VulkanTriangle)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

ExternalProject_Add(
    GLFW 
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_SHALLOW true
    INSTALL_DIR ${CMAKE_BINARY_DIR}/deps/glfw
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/glfw
               -DGLFW_BUILD_TESTS=false
               -DGLFW_BUILD_EXAMPLES=false
)

ExternalProject_Add(
    FMT
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_SHALLOW true
    INSTALL_DIR ${CMAKE_BINARY_DIR}/deps/fmt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/fmt
               -DFMT_TEST=false
)

ExternalProject_Add(
    VulkanLoader
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
    GIT_SHALLOW true
    INSTALL_DIR ${CMAKE_BINARY_DIR}/deps/vulkan-loader
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/vulkan-loader
               -DUPDATE_DEPS=true
)

add_executable(vulkan-triangle src/main.cpp)

target_include_directories(vulkan-triangle PRIVATE
    ${CMAKE_BINARY_DIR}/deps/fmt/include
    ${CMAKE_BINARY_DIR}/deps/glfw/include
    ${CMAKE_BINARY_DIR}/VulkanLoader-prefix/src/VulkanLoader/external/Vulkan-Headers/include
)

target_link_directories(vulkan-triangle PRIVATE
    ${CMAKE_BINARY_DIR}/deps/fmt/lib
    ${CMAKE_BINARY_DIR}/deps/glfw/lib
    ${CMAKE_BINARY_DIR}/deps/vulkan-loader/lib
)

target_link_libraries(vulkan-triangle glfw3 vulkan-1)

# Some libraries have different names for debug and release binaries :pouting_-
# cat:. And to make things worse, only on windows. And to make things even wor-
# se, this seems to work the complete opposite as it should.
if (WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        target_link_libraries(vulkan-triangle fmt)
    else()
        target_link_libraries(vulkan-triangle fmtd)
    endif()
endif()