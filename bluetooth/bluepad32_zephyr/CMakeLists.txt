#
# Copyright (c) 2025 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bluepad32_zephyr)

# Main application source
target_sources(app PRIVATE
    src/main.c
)

# Bluetooth layer
target_sources(app PRIVATE
    src/bluetooth/bt_gatt_client.c
)

# TODO: Uncomment as we implement these modules
# target_sources(app PRIVATE
#     src/bluetooth/bt_connection.c
#     src/bluetooth/bt_scan.c
# )
# 
# # Platform layer
# target_sources(app PRIVATE
#     src/platform/uni_platform_zephyr.c
# )
# 
# # Device management
# target_sources(app PRIVATE
#     src/device/uni_hid_device.c
# )
# 
# # Controller abstractions
# target_sources(app PRIVATE
#     src/controller/uni_controller.c
#     src/controller/uni_gamepad.c
#     src/controller/uni_mouse.c
#     src/controller/uni_keyboard.c
# )

# HID Parsers - conditionally compiled based on Kconfig
target_sources_ifdef(CONFIG_BLUEPAD32_PARSER_GENERIC app PRIVATE
    src/parser/uni_hid_parser_generic.c
)

target_sources_ifdef(CONFIG_BLUEPAD32_PARSER_DS4 app PRIVATE
    src/parser/uni_hid_parser_ds4.c
)

target_sources_ifdef(CONFIG_BLUEPAD32_PARSER_SWITCH app PRIVATE
    src/parser/uni_hid_parser_switch.c
)

target_sources_ifdef(CONFIG_BLUEPAD32_PARSER_XBOX app PRIVATE
    src/parser/uni_hid_parser_xboxone.c
)

target_sources_ifdef(CONFIG_BLUEPAD32_PARSER_MOUSE app PRIVATE
    src/parser/uni_hid_parser_mouse.c
)

target_sources_ifdef(CONFIG_BLUEPAD32_PARSER_KEYBOARD app PRIVATE
    src/parser/uni_hid_parser_keyboard.c
)

# Include directories
target_include_directories(app PRIVATE
    src
    src/bluetooth
    src/controller
    src/parser
    src/device
    src/platform
)
