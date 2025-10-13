/*
 * SPDX-License-Identifier: Apache-2.0
 * 
 * MSPI SD Card Test Application
 * Tests MSPI communication at 400kHz in single-line SPI mode
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/mspi.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mspi_sd_test, LOG_LEVEL_INF);

#define MSPI_NODE DT_NODELABEL(hpf_mspi0)

int main(void)
{
	const struct device *mspi_dev;
	int ret;

	LOG_INF("===========================================");
	LOG_INF("MSPI SD Card Test - Starting");
	LOG_INF("Testing MSPI at 400kHz in SPI mode");
	LOG_INF("===========================================");

	/* Get MSPI device */
	mspi_dev = DEVICE_DT_GET(MSPI_NODE);
	if (!device_is_ready(mspi_dev)) {
		LOG_ERR("MSPI device not ready!");
		return -1;
	}

	LOG_INF("MSPI device %s is ready", mspi_dev->name);

	/* Configure MSPI controller */
	struct mspi_dt_spec mspi_spec = MSPI_DT_SPEC_GET(MSPI_NODE);
	ret = mspi_config(&mspi_spec);
	if (ret < 0) {
		LOG_ERR("Failed to configure MSPI: %d", ret);
		return ret;
	}

	LOG_INF("MSPI configured successfully");

	/* Configure device settings for SD card */
	struct mspi_dev_id dev_id = {
		.dev_idx = 0,
	};

	struct mspi_dev_cfg dev_cfg = {
		.ce_num = 0,  /* CS on first CE line */
		.freq = 400000,  /* 400kHz for SD initialization */
		.io_mode = MSPI_IO_MODE_SINGLE,  /* Single-line SPI */
		.data_rate = MSPI_DATA_RATE_SINGLE,
		.cpp_mode = MSPI_CPP_MODE_0,  /* SPI Mode 0 */
		.endian = MSPI_XFER_LITTLE_ENDIAN,
		.ce_polarity = MSPI_CE_ACTIVE_LOW,
		.dqs_enable = false,
	};

	ret = mspi_dev_config(mspi_dev, &dev_id, MSPI_DEVICE_CONFIG_ALL, &dev_cfg);
	if (ret < 0) {
		LOG_ERR("Failed to configure MSPI device: %d", ret);
		return ret;
	}

	LOG_INF("MSPI device configured: 400kHz, Single-line, SPI Mode 0");

	/* Test data - send CMD0 (GO_IDLE_STATE) to SD card */
	uint8_t tx_data[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};  /* CMD0 with CRC */
	uint8_t rx_data[6] = {0};

	LOG_INF("Sending test command (CMD0) to SD card...");

	/* Prepare MSPI transfer */
	struct mspi_xfer xfer = {
		.async = false,
		.xfer_mode = MSPI_PIO,
		.tx_dummy = 0,
		.cmd_length = 0,  /* No separate command phase for SPI mode */
		.addr_length = 0,  /* No address phase */
		.num_packet = 1,
		.packets = (struct mspi_xfer_packet[]) {
			{
				.dir = MSPI_TX,
				.cmd = 0,
				.address = 0,
				.data_buf = tx_data,
				.num_bytes = sizeof(tx_data),
			},
		},
	};

	ret = mspi_transceive(mspi_dev, &dev_id, &xfer);
	if (ret < 0) {
		LOG_ERR("MSPI transceive failed: %d", ret);
		return ret;
	}

	LOG_INF("Command sent successfully!");
	LOG_HEXDUMP_INF(tx_data, sizeof(tx_data), "TX Data:");

	LOG_INF("===========================================");
	LOG_INF("MSPI Test Complete");
	LOG_INF("===========================================");

	/* Keep running to observe output */
	while (1) {
		k_sleep(K_SECONDS(5));
		LOG_INF("MSPI test running...");
	}

	return 0;
}
