# Copyright (c) 2024 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

# Add the remote (FLPR) application as a sysbuild image
ExternalZephyrProject_Add(
    APPLICATION remote
    SOURCE_DIR ${APP_DIR}/remote
    BOARD xiao_nrf54l15/nrf54l15/cpuflpr
)
