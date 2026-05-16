cmake_minimum_required(VERSION 3.10)
project(NuklearCherryUSB C)

set(CMAKE_C_STANDARD 99)

# Find SDL2
find_package(SDL2 REQUIRED)

# Include directories
include_directories(
    include
    include/external
    kernel
    kernel/cherryusb/core
    kernel/cherryusb/common
    kernel/cherryusb/class/hub
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
    kernel/usb_hal_ports.c
    kernel/vfs.c
    kernel/scheduler.c
    kernel/i18n.c
    kernel/uac_policy.c
    kernel/tgx_impl.c
    kernel/tlsf_impl.c
)

# Executable
add_executable(nuklear_cherry_usb ${SOURCES})

# Libraries
target_link_libraries(nuklear_cherry_usb
    ${SDL2_LIBRARIES}
    m
    dl
)
