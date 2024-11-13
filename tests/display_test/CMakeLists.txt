# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

# Define board root to find our custom shield
set(BOARD_ROOT ${CMAKE_SOURCE_DIR})

# Default board to blackpill
if (NOT DEFINED ${BOARD})
  set(BOARD blackpill_f411ce)
endif()

if (NOT DEFINED ${BOARD})
set(SHIELD sh1106)
#set(SHIELD ssd1306)
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Include some zephyr cmake extensions
include(${ZEPHYR_BASE}/cmake/modules/extensions.cmake)

project(display_test)

FILE(GLOB app_sources src/*.cpp)
target_sources(app PRIVATE ${app_sources})
