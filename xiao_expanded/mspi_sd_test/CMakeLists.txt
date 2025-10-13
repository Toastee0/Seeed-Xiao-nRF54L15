# SPDX-License-Identifier: Apache-2.0
# MSPI Test Application for SD Card

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mspi_sd_test)

target_sources(app PRIVATE src/main.c)
