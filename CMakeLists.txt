# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Force Release build
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()


# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

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
set(PICO_BOARD waveshare_rp2350_zero CACHE STRING "Board type")

# Automatically set platform based on board name
if (PICO_BOARD MATCHES "rp2350")
    set(PICO_PLATFORM rp2350 CACHE STRING "Platform type" FORCE)
else()
    set(PICO_PLATFORM rp2040 CACHE STRING "Platform type" FORCE)
endif()







# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(EMUZ80_MultiBoard C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# AE-RP2040
add_subdirectory(boards/AE-RP2040)

# RP2350-Zero
add_subdirectory(boards/RP2350-Zero)

# WeAct-RP2350B
add_subdirectory(boards/WeAct-RP2350B)
