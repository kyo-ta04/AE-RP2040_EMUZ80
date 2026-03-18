cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Force Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Initialise pico_sdk from installed location
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")
set(PICO_PLATFORM rp2040 CACHE STRING "Platform type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(EMUZ80_MultiBoard C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(AE-RP2040_EMUZ80 AE-RP2040_EMUZ80.c )

pico_set_program_name(AE-RP2040_EMUZ80 "AE-RP2040_EMUZ80")
pico_set_program_version(AE-RP2040_EMUZ80 "0.1")

# Generate PIO header
pico_generate_pio_header(AE-RP2040_EMUZ80 ${CMAKE_CURRENT_LIST_DIR}/bus.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(AE-RP2040_EMUZ80 0)
pico_enable_stdio_usb(AE-RP2040_EMUZ80 1)

# Add the standard library to the build
target_link_libraries(AE-RP2040_EMUZ80
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(AE-RP2040_EMUZ80 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(AE-RP2040_EMUZ80 
        hardware_pio
        hardware_timer
        hardware_pwm
        pico_multicore
        )

pico_add_extra_outputs(AE-RP2040_EMUZ80)

# Performance Optimizations
pico_set_binary_type(AE-RP2040_EMUZ80 copy_to_ram)
target_compile_options(AE-RP2040_EMUZ80 PRIVATE -O3)

