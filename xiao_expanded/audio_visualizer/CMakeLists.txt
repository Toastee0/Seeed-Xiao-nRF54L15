# SPDX-License-Identifier: Apache-2.0
set(BOARD_ROOT "$ENV{ZEPHYR_BASE}/../../platforms/Seeed Studio/zephyr")

cmake_minimum_required(VERSION 3.13.1)
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