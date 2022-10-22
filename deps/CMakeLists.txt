add_subdirectory(glfw)
add_subdirectory(fmt)

include(ExternalProject)

ExternalProject_Add(
    Vulkan-Headers
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Vulkan-Headers
    BINARY_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Headers
    INSTALL_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Headers/install
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/Vulkan-Headers/install
)

ExternalProject_Add(
    Vulkan-Loader
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Vulkan-Loader
    BINARY_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Loader
    INSTALL_DIR ${CMAKE_BINARY_DIR}/deps/Vulkan-Loader/install
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps/Vulkan-Loader/install
               -DVULKAN_HEADERS_INSTALL_DIR=${CMAKE_BINARY_DIR}/deps/Vulkan-Headers/install
               -DBUILD_SHARED_LIBS=false
)

if (UNIX AND NOT APPLE)
    set_property(TARGET vulkan PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/deps/Vulkan-Loader/install/lib/libvulkan.so)
endif()

ExternalProject_Add_StepDependencies(Vulkan-Loader build Vulkan-Headers)