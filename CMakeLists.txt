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

# Build for standard Raspberry Pi Pico (rp2040)
set(PICO_BOARD pico CACHE STRING "Board type" FORCE)
set(PICO_PLATFORM rp2040 CACHE STRING "Platform type" FORCE)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(AE-RP2040_EMUZ80 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

set(TARGET "AE-RP2040_EMUZ80")

add_executable(${TARGET} AE-RP2040.c)
pico_set_program_name(${TARGET} ${TARGET})
pico_set_program_version(${TARGET} "0.1")
pico_generate_pio_header(${TARGET} ${CMAKE_CURRENT_LIST_DIR}/AE-RP2040.pio)
pico_enable_stdio_uart(${TARGET} 0)
pico_enable_stdio_usb(${TARGET} 1)
target_link_libraries(${TARGET} pico_stdlib hardware_pio hardware_timer hardware_pwm pico_multicore)
target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
pico_add_extra_outputs(${TARGET})
pico_set_binary_type(${TARGET} copy_to_ram)
target_compile_options(${TARGET} PRIVATE -O3)
