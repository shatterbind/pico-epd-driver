# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion RISCV_ZCB_RPI_2_2_0_3)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_epd_driver C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_subdirectory(src/common)
include_directories(./src/common)

add_subdirectory(src/epd_2_13)
include_directories(./src/epd_2_13)

add_subdirectory(src/canvas)
include_directories(./src/canvas)

# Add executable. Default name is the project name, version 0.1
add_executable(${CMAKE_PROJECT_NAME} main.c)

# Set the program name and version
pico_set_program_name(${CMAKE_PROJECT_NAME} "pico_epd_driver")
pico_set_program_version(${CMAKE_PROJECT_NAME} "0.1")

# UART & USB outputs
# pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 1)
# pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)

# pull in common dependencies
target_link_libraries(${CMAKE_PROJECT_NAME} epd_2_13 canvas pico_stdlib hardware_adc)

pico_add_extra_outputs(${CMAKE_PROJECT_NAME})
