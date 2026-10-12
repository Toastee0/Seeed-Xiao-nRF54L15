# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
# Boot-time reset reason readout, see lib/reset_reason
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/../../lib/reset_reason)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bap_dmic)

target_sources(app PRIVATE
  src/main.c
)

if (CONFIG_USE_USB_AUDIO_INPUT)
  include(${ZEPHYR_BASE}/samples/subsys/usb/common/common.cmake)
endif()
