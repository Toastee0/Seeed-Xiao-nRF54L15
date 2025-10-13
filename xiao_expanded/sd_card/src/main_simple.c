/*
 * Copyright (c) 2019 Tavish Naruka <tavishnaruka@gmail.com>
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * Copyright (c) 2025 Seeed Technology Co.,Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* SD Card Filesystem Sample for XIAO nRF54L15 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>

#include <ff.h>

#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT "/SD:"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static FATFS fat_fs;
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

int main(void)
{
	int res;
	static const char *disk_pdrv = DISK_DRIVE_NAME;
	
	LOG_INF("========================================");
	LOG_INF("XIAO nRF54L15 SD Card Sample");
	LOG_INF("========================================");
	LOG_INF("Checking SD card...");

	/* Get SD card info (no init needed - happens automatically) */
	uint32_t block_count, block_size;
	if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_GET_SECTOR_COUNT, &block_count) == 0) {
		LOG_INF("Block count: %u", block_count);
	}
	if (disk_access_ioctl(disk_pdrv, DISK_IOCTL_GET_SECTOR_SIZE, &block_size) == 0) {
		LOG_INF("Block size: %u bytes", block_size);
		uint32_t size_mb = ((uint64_t)block_count * block_size) >> 20;
		LOG_INF("Total size: %u MB", size_mb);
	}

	/* Mount filesystem */
	mp.mnt_point = DISK_MOUNT_PT;
	res = fs_mount(&mp);
	if (res == FR_OK) {
		LOG_INF("Filesystem mounted at %s", DISK_MOUNT_PT);
		
		/* Create a test file */
		struct fs_file_t file;
		fs_file_t_init(&file);
		
		char path[64];
		snprintf(path, sizeof(path), "%s/test.txt", DISK_MOUNT_PT);
		
		res = fs_open(&file, path, FS_O_CREATE | FS_O_WRITE);
		if (res == 0) {
			const char *data = "Hello from XIAO nRF54L15!\n";
			fs_write(&file, data, strlen(data));
			fs_close(&file);
			LOG_INF("Created test file: %s", path);
		} else {
			LOG_WRN("Could not create test file (err %d)", res);
		}
		
		fs_unmount(&mp);
		LOG_INF("SD card unmounted");
	} else {
		LOG_ERR("Failed to mount filesystem (err %d)", res);
		LOG_ERR("Try formatting SD card as FAT32");
	}

	LOG_INF("========================================");
	LOG_INF("Sample completed");
	LOG_INF("========================================");

	while (1) {
		k_sleep(K_SECONDS(1));
	}
	return 0;
}
