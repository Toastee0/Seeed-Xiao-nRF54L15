# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(include)

if(CONFIG_XIAO_RESET_REASON)
  zephyr_library()
  zephyr_library_sources(reset_reason.c)
endif()
