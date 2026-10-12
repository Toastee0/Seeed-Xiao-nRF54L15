# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause

cmake_minimum_required(VERSION 3.13.1)
# Boot-time reset reason readout, see lib/reset_reason
list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/../../../../lib/reset_reason)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mesh_dfu_target)

set(dfu_common_dir ${ZEPHYR_NRF_MODULE_DIR}/samples/bluetooth/mesh/dfu/common/src)

include_directories(
	${dfu_common_dir}
	${ZEPHYR_NRF_MODULE_DIR}/samples/bluetooth/mesh/common
)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources}
	${dfu_common_dir}/dfu_target.c
)

target_sources_ifdef(CONFIG_NCS_SAMPLE_MCUMGR_BT_OTA_DFU app PRIVATE ${app_sources}
	${ZEPHYR_NRF_MODULE_DIR}/samples/bluetooth/mesh/common/smp_bt.c
)
