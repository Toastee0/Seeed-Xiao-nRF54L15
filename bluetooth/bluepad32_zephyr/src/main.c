/*
 * Copyright (c) 2025 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

LOG_MODULE_REGISTER(bluepad32, LOG_LEVEL_DBG);

/* External GATT client API */
extern int bt_gatt_client_init(void);
extern int bt_gatt_client_discover_hid_service(struct bt_conn *conn);

static bool scanning = false;
static struct bt_conn *default_conn = NULL;
static bool connecting = false;
static struct k_work_delayable pairing_work;

/* Forward declarations */
static void scan_start(void);
static void scan_stop(void);
static void start_pairing_work_handler(struct k_work *work);

/* Delayed work to start pairing after connection stabilizes */
static void start_pairing_work_handler(struct k_work *work)
{
	int err;

	if (!default_conn) {
		LOG_ERR("No active connection for pairing");
		return;
	}

	LOG_INF("Starting security upgrade to trigger pairing...");
	err = bt_conn_set_security(default_conn, BT_SECURITY_L2);
	if (err) {
		LOG_ERR("Failed to set security: %d", err);
	}
}

/* Bluetooth connection callbacks */
static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];
	struct bt_conn_info info;

	connecting = false;

	if (err) {
		LOG_ERR("Connection failed (err 0x%02x)", err);
		/* Resume scanning on connection failure */
		scan_start();
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Connected: %s", addr);

	/* Get connection info */
	bt_conn_get_info(conn, &info);
	LOG_INF("Connection interval: %u units (%.2f ms)", 
		info.le.interval, info.le.interval * 1.25);

	/* Store connection reference */
	default_conn = bt_conn_ref(conn);

	/* Stop scanning when connected */
	if (scanning) {
		scan_stop();
	}

	/* Let GATT discovery trigger encryption automatically when needed.
	 * Original Bluepad32 doesn't manually request pairing - the stack
	 * handles it automatically during GATT operations that require encryption.
	 * Start discovery immediately - security will be elevated as needed.
	 */
	LOG_INF("Starting GATT service discovery...");
	err = bt_gatt_client_discover_hid_service(conn);
	if (err) {
		LOG_ERR("Failed to start GATT discovery (err %d)", err);
		bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Disconnected: %s (reason 0x%02x)", addr, reason);

	/* Cancel any pending pairing work */
	k_work_cancel_delayable(&pairing_work);

	/* Release connection reference */
	if (default_conn) {
		bt_conn_unref(default_conn);
		default_conn = NULL;
	}

	connecting = false;

	/* Add delay before resuming scan to let stack clean up */
	k_sleep(K_MSEC(500));

	/* Resume scanning */
	scan_start();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

/* Security/Pairing callbacks */
static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Pairing completed: %s, bonded: %d", addr, bonded);

	/* GATT discovery already started in connected() callback.
	 * Pairing happens automatically when GATT operations require it.
	 */
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_ERR("Pairing failed: %s, reason %d", addr, reason);
	
	/* Disconnect on pairing failure */
	bt_conn_disconnect(conn, BT_HCI_ERR_AUTH_FAIL);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_WRN("Pairing cancelled: %s", addr);
}

static struct bt_conn_auth_cb auth_callbacks = {
	.cancel = auth_cancel,
};

static struct bt_conn_auth_info_cb auth_info_callbacks = {
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed,
};

/* Scan callback */
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];
	int err;

	bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	
	/* Filter: Only show devices starting with F4 (Xbox controllers) */
	if (addr->a.val[5] != 0xF4) {
		return;
	}

	LOG_INF("Xbox device found: %s (RSSI %d dBm)", addr_str, rssi);
	LOG_INF("  Address type: %s", addr->type == BT_ADDR_LE_PUBLIC ? "PUBLIC" : "RANDOM");
	LOG_INF("  AD type: 0x%02x, AD data length: %u bytes", type, ad->len);

	/* Skip if already connected or connecting */
	if (default_conn || connecting) {
		LOG_DBG("Already connected/connecting, ignoring");
		return;
	}

	/* Stop scanning before connecting */
	LOG_INF("Stopping scan to initiate connection...");
	scan_stop();

	/* Check if a stale connection exists - if so, skip this attempt */
	struct bt_conn *existing = bt_conn_lookup_addr_le(BT_ID_DEFAULT, addr);
	if (existing) {
		LOG_WRN("Stale connection still exists, skipping connection attempt");
		bt_conn_unref(existing);
		/* Resume scanning after delay */
		k_sleep(K_SECONDS(2));
		scan_start();
		return;
	}

	/* Initiate connection */
	LOG_INF("Connecting to Xbox controller...");
	connecting = true;
	
	struct bt_le_conn_param *param = BT_LE_CONN_PARAM_DEFAULT;
	struct bt_conn *conn = NULL;
	
	err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, param, &conn);
	if (err) {
		LOG_ERR("Failed to create connection (err %d)", err);
		connecting = false;
		/* Restart scanning on error */
		scan_start();
		return;
	}

	/* Store connection reference */
	default_conn = bt_conn_ref(conn);
	bt_conn_unref(conn);

	LOG_INF("Connection request sent");
}

static void scan_start(void)
{
	int err;

	if (scanning) {
		return;
	}

	/* Bluepad32 uses passive scanning with interval=window=48 (30ms)
	 * Passive scanning doesn't send scan requests, more battery efficient
	 */
	struct bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_PASSIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = 0x0030,  /* 48 * 0.625ms = 30ms */
		.window     = 0x0030,  /* 48 * 0.625ms = 30ms */
	};

	LOG_DBG("Scan parameters: type=%s interval=%u window=%u",
		scan_param.type == BT_LE_SCAN_TYPE_ACTIVE ? "ACTIVE" : "PASSIVE",
		scan_param.interval, scan_param.window);

	err = bt_le_scan_start(&scan_param, device_found);
	if (err) {
		LOG_ERR("Scanning failed to start (err %d)", err);
		return;
	}

	scanning = true;
	LOG_INF("Scanning started");
	LOG_DBG("Looking for BLE HID devices (game controllers)...");
}

static void scan_stop(void)
{
	int err;

	if (!scanning) {
		return;
	}

	err = bt_le_scan_stop();
	if (err) {
		LOG_ERR("Failed to stop scanning (err %d)", err);
		return;
	}

	scanning = false;
	LOG_INF("Scanning stopped");
}

int main(void)
{
	int err;

	LOG_INF("Bluepad32 for Zephyr starting...");

	/* Initialize delayed work for pairing */
	k_work_init_delayable(&pairing_work, start_pairing_work_handler);

	/* Initialize Bluetooth subsystem */
	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return 0;
	}

	/* Register authentication callbacks */
	err = bt_conn_auth_cb_register(&auth_callbacks);
	if (err) {
		LOG_ERR("Failed to register auth callbacks (err %d)", err);
		return 0;
	}

	err = bt_conn_auth_info_cb_register(&auth_info_callbacks);
	if (err) {
		LOG_ERR("Failed to register auth info callbacks (err %d)", err);
		return 0;
	}

	LOG_INF("Bluetooth initialized");
	
	/* Initialize GATT client module */
	err = bt_gatt_client_init();
	if (err) {
		LOG_ERR("GATT client init failed (err %d)", err);
		return 0;
	}
	
	LOG_INF("Starting BLE scan for HID devices...");

	/* Start scanning for HID devices */
	scan_start();

	/* Main loop */
	while (1) {
		k_sleep(K_SECONDS(1));
		/* TODO: Process controller input */
	}

	return 0;
}
