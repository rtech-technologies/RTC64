cmake_minimum_required(VERSION 3.10)
project(NuklearCherryUSB C)

set(CMAKE_C_STANDARD 99)

# Find SDL2
find_package(SDL2 REQUIRED)

# Include directories
include_directories(
    include
    kernel
    external/CherryUSB/core
    external/CherryUSB/common
    external/CherryUSB/class/hub
    ${SDL2_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.c
    src/app_ui.c
    src/nuklear_impl.c
    src/nk_software_renderer.c
    kernel/storage.c
    kernel/input.c
    kernel/usb_stubs.c
)

# Executable
add_executable(nuklear_cherry_usb ${SOURCES})

# Libraries
target_link_libraries(nuklear_cherry_usb
    ${SDL2_LIBRARIES}
    m
    dl
)
