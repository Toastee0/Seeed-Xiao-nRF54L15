# SPDX-License-Identifier: Apache-2.0
# SPDX-FileCopyrightText: 2024 Nordic Semiconductor ASA

cmake_minimum_required(VERSION 3.20.0)

# Boot-time reset reason readout, see lib/reset_reason
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/../../lib/reset_reason)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(hpf_ws2812_bitbang)

# Simple cpuapp blink - WS2812 runs on FLPR remote core
target_sources(app PRIVATE src/main.c)