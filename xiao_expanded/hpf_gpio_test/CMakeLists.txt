# SPDX-License-Identifier: Apache-2.0
# Blinky with HPF GPIO Test

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hpf_gpio_blinky)

target_sources(app PRIVATE src/main.c)
