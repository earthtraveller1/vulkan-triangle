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

add_subdirectory(deps/glfw)
add_subdirectory(deps/fmt)

include(ExternalProject)

ExternalProject_Add(
    Vulkan-Headers
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/Vulkan-Headers
    BINARY_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Headers
    INSTALL_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Headers/install
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/Vulkan-Headers/install
)

ExternalProject_Add(
    Vulkan-Loader
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/Vulkan-Loader
    BINARY_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Loader
    INSTALL_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Loader/install
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/Vulkan-Loader/install
               -DVULKAN_HEADERS_INSTALL_DIR=${CMAKE_BINARY_DIR}/deps/Vulkan-Headers/install
               -DBUILD_SHARED_LIBS=false
)

if (UNIX AND NOT APPLE)
    set(VULKAN_LINK_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Loader/install/lib)
    set(VULKAN_LIB vulkan)
endif()

ExternalProject_Add_StepDependencies(Vulkan-Loader build Vulkan-Headers)

add_executable(vulkan-triangle src/main.cpp)

add_dependencies(vulkan-triangle Vulkan-Loader)

target_include_directories(vulkan-triangle PRIVATE
    ${CMAKE_SOURCE_DIR}/deps/fmt/include
    ${CMAKE_SOURCE_DIR}/deps/glfw/include
    ${CMAKE_SOURCE_DIR}/deps/Vulkan-Headers/include
)

target_link_directories(vulkan-triangle PRIVATE
    ${VULKAN_LINK_DIR}
)

target_link_libraries(vulkan-triangle glfw fmt ${VULKAN_LIB})