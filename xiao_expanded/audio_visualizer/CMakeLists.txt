# SPDX-License-Identifier: Apache-2.0
set(BOARD_ROOT "$ENV{ZEPHYR_BASE}/../../platforms/Seeed Studio/zephyr")

cmake_minimum_required(VERSION 3.13.1)
# Boot-time reset reason readout, see lib/reset_reason
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/../../lib/reset_reason)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(audio_visualizer)

file(GLOB U8G2_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/u8g2/csrc/*.c
)

target_sources(app
  PRIVATE
    src/main.c
    ${U8G2_SOURCES}
)

target_include_directories(app 
  PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/u8g2/csrc
)