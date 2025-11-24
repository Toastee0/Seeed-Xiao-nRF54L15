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
#include <zephyr/bluetooth/uuid.h>

LOG_MODULE_REGISTER(bt_gatt_client, LOG_LEVEL_DBG);

/* Use Zephyr's predefined HID UUIDs */
/* BT_UUID_HIDS is already defined as 0x1812 */
/* BT_UUID_HIDS_REPORT_MAP is already defined as 0x2A4B */
/* BT_UUID_HIDS_REPORT is already defined as 0x2A4D */
/* BT_UUID_HIDS_INFO is already defined as 0x2A4A */
/* BT_UUID_HIDS_CTRL_POINT is already defined as 0x2A4C */

/* Report Descriptor size limit */
#define REPORT_MAP_MAX_SIZE 512

/* GATT discovery state machine */
typedef enum {
	DISC_STATE_IDLE,
	DISC_STATE_SERVICE,
	DISC_STATE_CHARACTERISTICS,
	DISC_STATE_DESCRIPTORS,
	DISC_STATE_COMPLETE
} disc_state_t;

/* Per-connection GATT state */
struct gatt_client_ctx {
	struct bt_conn *conn;
	disc_state_t state;
	
	/* Discovered handles */
	uint16_t hid_service_handle_start;
	uint16_t hid_service_handle_end;
	uint16_t report_map_handle;
	uint16_t report_handle;
	uint16_t report_ccc_handle;
	
	/* Report Map storage */
	uint8_t report_map[REPORT_MAP_MAX_SIZE];
	uint16_t report_map_len;
	uint16_t report_map_offset;
};

static struct gatt_client_ctx gatt_ctx;

/* Forward declarations */
static uint8_t discover_hid_service_cb(struct bt_conn *conn,
				       const struct bt_gatt_attr *attr,
				       struct bt_gatt_discover_params *params);
static uint8_t discover_characteristics_cb(struct bt_conn *conn,
					   const struct bt_gatt_attr *attr,
					   struct bt_gatt_discover_params *params);
static uint8_t read_report_map_cb(struct bt_conn *conn, uint8_t err,
				  struct bt_gatt_read_params *params,
				  const void *data, uint16_t length);
static void subscribe_report_cb(struct bt_conn *conn, uint8_t err,
				struct bt_gatt_subscribe_params *params);
static uint8_t report_notify_cb(struct bt_conn *conn,
				struct bt_gatt_subscribe_params *params,
				const void *data, uint16_t length);

/* Discovery parameters */
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_read_params read_params;
static struct bt_gatt_subscribe_params subscribe_params;

/* HID Service discovery callback */
static uint8_t discover_hid_service_cb(struct bt_conn *conn,
				       const struct bt_gatt_attr *attr,
				       struct bt_gatt_discover_params *params)
{
	int err;

	if (!attr) {
		LOG_INF("HID service discovery complete");
		LOG_INF("HID Service found: handle range %u-%u",
			gatt_ctx.hid_service_handle_start,
			gatt_ctx.hid_service_handle_end);
		
		/* Start characteristic discovery */
		gatt_ctx.state = DISC_STATE_CHARACTERISTICS;
		
		memset(params, 0, sizeof(*params));
		params->uuid = NULL; /* Discover all characteristics */
		params->start_handle = gatt_ctx.hid_service_handle_start;
		params->end_handle = gatt_ctx.hid_service_handle_end;
		params->type = BT_GATT_DISCOVER_CHARACTERISTIC;
		params->func = discover_characteristics_cb;

		err = bt_gatt_discover(conn, params);
		if (err) {
			LOG_ERR("Characteristic discovery failed (err %d)", err);
		}
		
		return BT_GATT_ITER_STOP;
	}

	struct bt_gatt_service_val *svc_val = (struct bt_gatt_service_val *)attr->user_data;
	
	LOG_INF("HID Service discovered at handle %u", attr->handle);
	gatt_ctx.hid_service_handle_start = attr->handle + 1;
	gatt_ctx.hid_service_handle_end = svc_val->end_handle;

	return BT_GATT_ITER_CONTINUE;
}

/* Characteristics discovery callback */
static uint8_t discover_characteristics_cb(struct bt_conn *conn,
					   const struct bt_gatt_attr *attr,
					   struct bt_gatt_discover_params *params)
{
	int err;

	if (!attr) {
		LOG_INF("Characteristic discovery complete");
		LOG_INF("Report Map handle: %u", gatt_ctx.report_map_handle);
		LOG_INF("Report handle: %u", gatt_ctx.report_handle);
		
		/* Check if we found the essential characteristics */
		if (gatt_ctx.report_map_handle == 0) {
			LOG_ERR("Report Map characteristic not found");
			return BT_GATT_ITER_STOP;
		}
		
		/* Start reading the Report Map */
		memset(&read_params, 0, sizeof(read_params));
		read_params.func = read_report_map_cb;
		read_params.handle_count = 1;
		read_params.single.handle = gatt_ctx.report_map_handle;
		read_params.single.offset = 0;
		
		gatt_ctx.report_map_offset = 0;
		gatt_ctx.report_map_len = 0;
		
		err = bt_gatt_read(conn, &read_params);
		if (err) {
			LOG_ERR("Failed to read Report Map (err %d)", err);
		} else {
			LOG_INF("Reading HID Report Descriptor...");
		}
		
		return BT_GATT_ITER_STOP;
	}

	struct bt_gatt_chrc *chrc_val = (struct bt_gatt_chrc *)attr->user_data;
	
	if (bt_uuid_cmp(chrc_val->uuid, BT_UUID_HIDS_REPORT_MAP) == 0) {
		LOG_INF("Report Map characteristic found at handle %u", chrc_val->value_handle);
		gatt_ctx.report_map_handle = chrc_val->value_handle;
	} else if (bt_uuid_cmp(chrc_val->uuid, BT_UUID_HIDS_REPORT) == 0) {
		LOG_INF("Report characteristic found at handle %u (properties: 0x%02x)",
			chrc_val->value_handle, chrc_val->properties);
		/* Store the first Report characteristic for input reports */
		if (gatt_ctx.report_handle == 0) {
			gatt_ctx.report_handle = chrc_val->value_handle;
		}
	} else if (bt_uuid_cmp(chrc_val->uuid, BT_UUID_HIDS_INFO) == 0) {
		LOG_INF("HID Information characteristic found at handle %u", chrc_val->value_handle);
	} else if (bt_uuid_cmp(chrc_val->uuid, BT_UUID_HIDS_CTRL_POINT) == 0) {
		LOG_INF("HID Control Point characteristic found at handle %u", chrc_val->value_handle);
	}

	return BT_GATT_ITER_CONTINUE;
}

/* Report Map read callback */
static uint8_t read_report_map_cb(struct bt_conn *conn, uint8_t err,
				  struct bt_gatt_read_params *params,
				  const void *data, uint16_t length)
{
	if (err) {
		LOG_ERR("Report Map read failed (err %u)", err);
		return BT_GATT_ITER_STOP;
	}

	if (!data) {
		/* Read complete */
		LOG_INF("Report Map read complete: %u bytes", gatt_ctx.report_map_len);
		LOG_HEXDUMP_INF(gatt_ctx.report_map, gatt_ctx.report_map_len, "HID Report Descriptor:");
		
		/* Subscribe to HID Report notifications
		 * bt_gatt_subscribe() automatically handles CCC descriptor discovery
		 */
		if (gatt_ctx.report_handle > 0) {
			memset(&subscribe_params, 0, sizeof(subscribe_params));
			subscribe_params.notify = report_notify_cb;
			subscribe_params.value = BT_GATT_CCC_NOTIFY;
			subscribe_params.value_handle = gatt_ctx.report_handle;
			subscribe_params.subscribe = subscribe_report_cb;
			
			int ret = bt_gatt_subscribe(conn, &subscribe_params);
			if (ret) {
				LOG_ERR("Failed to subscribe to Reports (err %d)", ret);
				gatt_ctx.state = DISC_STATE_COMPLETE;
			} else {
				LOG_INF("Subscribing to HID Input Reports at handle %u...", 
					gatt_ctx.report_handle);
			}
		} else {
			LOG_WRN("No Report characteristic found for subscription");
			gatt_ctx.state = DISC_STATE_COMPLETE;
		}
		
		return BT_GATT_ITER_STOP;
	}

	/* Append data to report map buffer */
	if (gatt_ctx.report_map_len + length > REPORT_MAP_MAX_SIZE) {
		LOG_ERR("Report Map too large");
		return BT_GATT_ITER_STOP;
	}

	memcpy(&gatt_ctx.report_map[gatt_ctx.report_map_len], data, length);
	gatt_ctx.report_map_len += length;
	gatt_ctx.report_map_offset += length;

	LOG_DBG("Report Map fragment: %u bytes (total: %u)", length, gatt_ctx.report_map_len);

	return BT_GATT_ITER_CONTINUE;
}

/* Subscription callback */
static void subscribe_report_cb(struct bt_conn *conn, uint8_t err,
				struct bt_gatt_subscribe_params *params)
{
	if (err) {
		LOG_ERR("Subscribe failed (err %u)", err);
		return;
	}

	if (!params) {
		LOG_WRN("Unsubscribed from Reports");
		return;
	}

	LOG_INF("Subscribed to HID Reports successfully");
	LOG_INF("GATT discovery and setup complete - ready to receive controller input");
	gatt_ctx.state = DISC_STATE_COMPLETE;
}

/* Report notification callback */
static uint8_t report_notify_cb(struct bt_conn *conn,
				struct bt_gatt_subscribe_params *params,
				const void *data, uint16_t length)
{
	if (!data) {
		LOG_WRN("Unsubscribed from Report notifications");
		return BT_GATT_ITER_STOP;
	}
	
	LOG_INF("HID Report received: %u bytes", length);
	LOG_HEXDUMP_DBG(data, length, "Report data:");
	
	/* TODO: Parse Xbox controller input report */
	/* This is where we would call the HID parser to extract button states,
	 * analog stick positions, triggers, etc.
	 */

	return BT_GATT_ITER_CONTINUE;
}

/* Public API: Start GATT service discovery */
int bt_gatt_client_discover_hid_service(struct bt_conn *conn)
{
	int err;

	if (!conn) {
		return -EINVAL;
	}

	if (gatt_ctx.conn != NULL && gatt_ctx.state != DISC_STATE_IDLE) {
		LOG_WRN("Discovery already in progress");
		return -EBUSY;
	}

	LOG_INF("Starting HID service discovery");

	/* Initialize context */
	memset(&gatt_ctx, 0, sizeof(gatt_ctx));
	gatt_ctx.conn = conn;
	gatt_ctx.state = DISC_STATE_SERVICE;

	/* Start service discovery */
	memset(&discover_params, 0, sizeof(discover_params));
	discover_params.uuid = BT_UUID_HIDS;
	discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
	discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
	discover_params.type = BT_GATT_DISCOVER_PRIMARY;
	discover_params.func = discover_hid_service_cb;

	err = bt_gatt_discover(conn, &discover_params);
	if (err) {
		LOG_ERR("Service discovery failed (err %d)", err);
		gatt_ctx.state = DISC_STATE_IDLE;
		return err;
	}

	return 0;
}

int bt_gatt_client_init(void)
{
	LOG_DBG("GATT client module initialized");
	memset(&gatt_ctx, 0, sizeof(gatt_ctx));
	return 0;
}
