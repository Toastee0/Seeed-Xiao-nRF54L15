# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 Seeed Technology Co.,Ltd

cmake_minimum_required(VERSION 3.20.0)

# Boot-time reset reason readout, see lib/reset_reason
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/../../lib/reset_reason)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sd_card)

target_sources(app PRIVATE
  src/main.c
)
