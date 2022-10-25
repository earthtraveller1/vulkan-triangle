cmake_minimum_required(VERSION 3.13)

project(VulkanTriangle)

if (APPLE)
    message(FATAL_ERROR "Apple platforms are not supported!")
endif()

set(CMAKE_CXX_STANDARD 20)

option(VULKAN_TRIANGLE_COMPILE_SHADERS OFF)

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

function(compile_shader input output)
    add_custom_command(
        OUTPUT ${output} 
        COMMAND glslc 
        ARGS -o ${CMAKE_SOURCE_DIR}/${output} ${CMAKE_SOURCE_DIR}/${input}
        MAIN_DEPENDENCY ${input}
    )
endfunction()

set(VULKAN_LINK_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Loader/install/lib)
if (UNIX AND NOT APPLE)
    set(VULKAN_LIB vulkan)
elseif(WIN32)
    set(VULKAN_LIB vulkan-1)
endif()

ExternalProject_Add_StepDependencies(Vulkan-Loader build Vulkan-Headers)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -pedantic)
endif()

add_executable(vulkan-triangle 
    src/main.cpp
)

if (VULKAN_TRIANGLE_COMPILE_SHADERS)
    target_sources(vulkan-triangle PRIVATE
        shaders/shader.frag
        shaders/shader.vert)
    
    compile_shader(shaders/shader.vert shaders/shader.vert.spv)
    compile_shader(shaders/shader.frag shaders/shader.frag.spv)
endif()

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