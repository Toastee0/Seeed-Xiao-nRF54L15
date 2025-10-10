#
# Copyright (c) 2025 Seeed Technology Co.,Ltd
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyr_sdcard_ncs)

target_sources(app PRIVATE src/main.c)
