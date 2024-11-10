# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Default board to nodemcu esp32 wroom
if (NOT DEFINED ${BOARD})
  set(BOARD esp_devkitc_wroom)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Include some zephyr cmake extensions
include(${ZEPHYR_BASE}/cmake/modules/extensions.cmake)

# Define board root to find our custom shield
set(BOARD_ROOT ${CMAKE_SOURCE_DIR})

# Add custom drivers directory
add_subdirectory(drivers)

#set(SHIELD kitronik_crb_5693)

project(gpscompass)

set(driver_sources "")
FILE(GLOB app_sources src/*.cpp)
target_sources(app PRIVATE ${app_sources} ${driver_sources})
