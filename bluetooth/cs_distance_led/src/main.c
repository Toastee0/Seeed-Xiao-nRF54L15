/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Channel Sounding Initiator with LED Distance Indicator
 */

#include <math.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/bluetooth/cs.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/conn.h>
#include <bluetooth/scan.h>
#include <bluetooth/services/ras.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/cs_de.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led_strip.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_main, LOG_LEVEL_INF);

/* LED strip device */
#define STRIP_NODE DT_ALIAS(led_strip)
static const struct device *const led_strip = DEVICE_DT_GET(STRIP_NODE);
static struct led_rgb pixel = {0};

/* LED update control */
static bool bt_connected = false;
static float current_display_distance = 0.0f;
static uint32_t led_pulse_step = 0;

/* Calibration factors derived from measurements at 0.5m, 1m, 2m, 3m, and 4m
 * Formula: Actual_distance = (Raw_measurement - OFFSET) / SLOPE
 * 
 * Weighting strategy based on stability testing:
 * - Phase Slope: Most consistent across range (Weight: 50% - 2:1 ratio)
 * - RTT: Stable but noisier (Weight: 25%)
 * - IFFT: Phase ambiguity issues (Weight: 25%)
 */

/* Phase Slope calibration: Actual = (Phase_raw - 1.307) / 1.786 */
/* Most stable and consistent method - primary distance estimator */
#define PHASE_SLOPE_SLOPE   (1.786f)
#define PHASE_SLOPE_OFFSET_M (1.307f)
#define PHASE_WEIGHT        (0.50f)    /* Highest weight - 2:1 ratio vs others */

/* RTT calibration: Actual = (RTT_raw - 3.200) / 1.800 */
#define RTT_SLOPE           (1.800f)
#define RTT_OFFSET_M        (3.200f)
#define RTT_WEIGHT          (0.25f)

/* IFFT calibration: Complex non-linear, use best-fit approximation */
/* At short range (< 2m): ~1.75m offset, at long range more linear */
#define IFFT_SLOPE          (1.600f)   /* Approximate slope for calibration */
#define IFFT_OFFSET_M       (1.400f)   /* Approximate offset */
#define IFFT_WEIGHT         (0.25f)

/* LED control for Xiao nRF54L15 - Keep onboard LED for status */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

static inline int dk_leds_init(void) {
	int err;
	if (!gpio_is_ready_dt(&led0)) {
		LOG_ERR("LED device not ready");
		return -ENODEV;
	}
	err = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
	if (err < 0) {
		LOG_ERR("Failed to configure LED (err %d)", err);
		return err;
	}
	return 0;
}

static inline void dk_set_led_on(uint8_t led) {
	(void)led; /* Ignore LED number, we only have one */
	gpio_pin_set_dt(&led0, 1);
}

static inline void dk_set_led_off(uint8_t led) {
	(void)led; /* Ignore LED number, we only have one */
	gpio_pin_set_dt(&led0, 0);
}

#define CON_STATUS_LED 0

/* LED Strip Distance Indicator Functions */

/* Map distance to LED colors based on specifications:
 * 0-5m: Green brightest
 * 5-10m: Blue brightest
 */
static void distance_to_led_color(float distance_m)
{
	if (distance_m < 0.1f) {
		/* No valid distance, turn off */
		pixel.r = 0;
		pixel.g = 0;
		pixel.b = 0;
		return;
	}
	
	/* Red is always off when connected */
	pixel.r = 0;
	
	if (distance_m <= 5.0f) {
		/* 0-5m range: Green dominant */
		float green_ratio = 1.0f - (distance_m / 5.0f);
		pixel.g = (uint8_t)(255.0f * green_ratio);
		
		/* Blue: increases toward 5m */
		float blue_ratio = distance_m / 5.0f;
		pixel.b = (uint8_t)(128.0f * blue_ratio);
		
	} else if (distance_m <= 10.0f) {
		/* 5-10m range: Blue dominant */
		float green_ratio = 1.0f - ((distance_m - 5.0f) / 5.0f);
		pixel.g = (uint8_t)(128.0f * green_ratio);
		
		/* Blue: max at 10m */
		float blue_ratio = (distance_m - 5.0f) / 5.0f;
		pixel.b = (uint8_t)(128.0f + (127.0f * blue_ratio));
		
	} else {
		/* Beyond 10m: pure blue at max */
		pixel.g = 0;
		pixel.b = 255;
	}
}

/* Pulse red LED for disconnected state (0-128 over 1 second) */
static void pulse_red_led(void)
{
	const uint32_t steps_per_cycle = 20; /* 1000ms / 50ms = 20 steps */
	uint32_t position = led_pulse_step % steps_per_cycle;
	
	float phase = (float)position / steps_per_cycle;
	if (phase < 0.5f) {
		/* Rising: 0 to 128 */
		pixel.r = (uint8_t)(256.0f * phase);
	} else {
		/* Falling: 128 to 0 */
		pixel.r = (uint8_t)(256.0f * (1.0f - phase));
	}
	
	pixel.g = 0;
	pixel.b = 0;
	led_pulse_step++;
}

/* Update LED strip */
static void update_led_strip(void)
{
	if (!device_is_ready(led_strip)) {
		return;
	}
	
	if (bt_connected) {
		distance_to_led_color(current_display_distance);
	} else {
		pulse_red_led();
	}
	
	int rc = led_strip_update_rgb(led_strip, &pixel, 1);
	if (rc) {
		LOG_ERR("LED strip update failed: %d", rc);
	}
}

/* LED update thread */
static void led_update_thread(void)
{
	if (!device_is_ready(led_strip)) {
		LOG_ERR("LED strip device not ready");
		return;
	}
	
	LOG_INF("LED strip ready - distance indicator active");
	
	while (1) {
		update_led_strip();
		k_sleep(K_MSEC(50));
	}
}

K_THREAD_STACK_DEFINE(led_thread_stack, 1024);
static struct k_thread led_thread_data;

#define CS_CONFIG_ID	       0
#define NUM_MODE_0_STEPS       3
#define PROCEDURE_COUNTER_NONE (-1)
#define DE_SLIDING_WINDOW_SIZE (9)
#define MAX_AP		       (CONFIG_BT_RAS_MAX_ANTENNA_PATHS)

#define LOCAL_PROCEDURE_MEM                                                                        \
	((BT_RAS_MAX_STEPS_PER_PROCEDURE * sizeof(struct bt_le_cs_subevent_step)) +                \
	 (BT_RAS_MAX_STEPS_PER_PROCEDURE * BT_RAS_MAX_STEP_DATA_LEN))

static K_SEM_DEFINE(sem_remote_capabilities_obtained, 0, 1);
static K_SEM_DEFINE(sem_config_created, 0, 1);
static K_SEM_DEFINE(sem_cs_security_enabled, 0, 1);
static K_SEM_DEFINE(sem_connected, 0, 1);
static K_SEM_DEFINE(sem_discovery_done, 0, 1);
static K_SEM_DEFINE(sem_mtu_exchange_done, 0, 1);
static K_SEM_DEFINE(sem_security, 0, 1);
static K_SEM_DEFINE(sem_ras_features, 0, 1);
static K_SEM_DEFINE(sem_local_steps, 1, 1);
static K_SEM_DEFINE(sem_distance_estimate_updated, 0, 1);

static K_MUTEX_DEFINE(distance_estimate_buffer_mutex);

static struct bt_conn *connection;
NET_BUF_SIMPLE_DEFINE_STATIC(latest_local_steps, LOCAL_PROCEDURE_MEM);
NET_BUF_SIMPLE_DEFINE_STATIC(latest_peer_steps, BT_RAS_PROCEDURE_MEM);
static int32_t most_recent_local_ranging_counter = PROCEDURE_COUNTER_NONE;
static int32_t dropped_ranging_counter = PROCEDURE_COUNTER_NONE;
static uint32_t ras_feature_bits;

static uint8_t buffer_index;
static uint8_t buffer_num_valid;
static cs_de_dist_estimates_t distance_estimate_buffer[MAX_AP][DE_SLIDING_WINDOW_SIZE];
static struct bt_conn_le_cs_config cs_config;

static void store_distance_estimates(cs_de_report_t *p_report)
{
	int lock_state = k_mutex_lock(&distance_estimate_buffer_mutex, K_FOREVER);

	__ASSERT_NO_MSG(lock_state == 0);

	for (uint8_t ap = 0; ap < p_report->n_ap; ap++) {
		memcpy(&distance_estimate_buffer[ap][buffer_index],
		       &p_report->distance_estimates[ap], sizeof(cs_de_dist_estimates_t));
	}

	buffer_index = (buffer_index + 1) % DE_SLIDING_WINDOW_SIZE;

	if (buffer_num_valid < DE_SLIDING_WINDOW_SIZE) {
		buffer_num_valid++;
	}

	k_mutex_unlock(&distance_estimate_buffer_mutex);
}

static int float_cmp(const void *a, const void *b)
{
	float fa = *(const float *)a;
	float fb = *(const float *)b;

	return (fa > fb) - (fa < fb);
}

static float median_inplace(int count, float *values)
{
	if (count == 0) {
		return NAN;
	}

	qsort(values, count, sizeof(float), float_cmp);

	if (count % 2 == 0) {
		return (values[count/2] + values[count/2 - 1]) / 2;
	} else {
		return values[count/2];
	}
}

static cs_de_dist_estimates_t get_distance(uint8_t ap)
{
	cs_de_dist_estimates_t averaged_result = {};
	uint8_t num_ifft = 0;
	uint8_t num_phase_slope = 0;
	uint8_t num_rtt = 0;

	static float temp_ifft[DE_SLIDING_WINDOW_SIZE];
	static float temp_phase_slope[DE_SLIDING_WINDOW_SIZE];
	static float temp_rtt[DE_SLIDING_WINDOW_SIZE];

	int lock_state = k_mutex_lock(&distance_estimate_buffer_mutex, K_FOREVER);

	__ASSERT_NO_MSG(lock_state == 0);

	for (uint8_t i = 0; i < buffer_num_valid; i++) {
		if (isfinite(distance_estimate_buffer[ap][i].ifft)) {
			temp_ifft[num_ifft] = distance_estimate_buffer[ap][i].ifft;
			num_ifft++;
		}
		if (isfinite(distance_estimate_buffer[ap][i].phase_slope)) {
			temp_phase_slope[num_phase_slope] =
				distance_estimate_buffer[ap][i].phase_slope;
			num_phase_slope++;
		}
		if (isfinite(distance_estimate_buffer[ap][i].rtt)) {
			temp_rtt[num_rtt] = distance_estimate_buffer[ap][i].rtt;
			num_rtt++;
		}
	}

	k_mutex_unlock(&distance_estimate_buffer_mutex);

	averaged_result.ifft = median_inplace(num_ifft, temp_ifft);
	averaged_result.phase_slope = median_inplace(num_phase_slope, temp_phase_slope);
	averaged_result.rtt = median_inplace(num_rtt, temp_rtt);

	/* Apply calibration: Actual = (Raw - Offset) / Slope 
	 * Store calibrated values back into the result structure
	 */
	float ifft_calibrated = 0.0f;
	float phase_calibrated = 0.0f;
	float rtt_calibrated = 0.0f;
	
	bool ifft_valid = false;
	bool phase_valid = false;
	bool rtt_valid = false;

	/* Calibrate IFFT - ignore zeros and invalid values */
	if (isfinite(averaged_result.ifft) && averaged_result.ifft > 0.01f) {
		ifft_calibrated = (averaged_result.ifft - IFFT_OFFSET_M) / IFFT_SLOPE;
		if (ifft_calibrated < 0.0f) ifft_calibrated = 0.0f;
		ifft_valid = true;
		averaged_result.ifft = ifft_calibrated;
	}

	/* Calibrate Phase Slope - ignore zeros and invalid values */
	if (isfinite(averaged_result.phase_slope) && averaged_result.phase_slope > 0.01f) {
		phase_calibrated = (averaged_result.phase_slope - PHASE_SLOPE_OFFSET_M) / PHASE_SLOPE_SLOPE;
		if (phase_calibrated < 0.0f) phase_calibrated = 0.0f;
		phase_valid = true;
		averaged_result.phase_slope = phase_calibrated;
	}

	/* Calibrate RTT - ignore zeros and invalid values */
	if (isfinite(averaged_result.rtt) && averaged_result.rtt > 0.01f) {
		rtt_calibrated = (averaged_result.rtt - RTT_OFFSET_M) / RTT_SLOPE;
		if (rtt_calibrated < 0.0f) rtt_calibrated = 0.0f;
		rtt_valid = true;
		averaged_result.rtt = rtt_calibrated;
	}

	return averaged_result;
}

static void ranging_data_cb(struct bt_conn *conn, uint16_t ranging_counter, int err)
{
	ARG_UNUSED(conn);

	if (err) {
		LOG_ERR("Error when receiving ranging data with ranging counter %d (err %d)",
			ranging_counter, err);
		return;
	}

	if (ranging_counter != most_recent_local_ranging_counter) {
		LOG_INF("Ranging data dropped as peer ranging counter doesn't match local ranging "
			"data counter. (peer: %u, local: %u)",
			ranging_counter, most_recent_local_ranging_counter);
		net_buf_simple_reset(&latest_local_steps);
		k_sem_give(&sem_local_steps);
		return;
	}

	LOG_DBG("Ranging data received for ranging counter %d", ranging_counter);

	if (latest_local_steps.len == 0) {
		LOG_WRN("All subevents in ranging counter %u were aborted",
			most_recent_local_ranging_counter);
		net_buf_simple_reset(&latest_local_steps);
		k_sem_give(&sem_local_steps);

		if (!(ras_feature_bits & RAS_FEAT_REALTIME_RD)) {
			net_buf_simple_reset(&latest_peer_steps);
		}
		return;
	}

	/* This struct is static to avoid putting it on the stack (it's very large) */
	static cs_de_report_t cs_de_report;

	cs_de_populate_report(&latest_local_steps, &latest_peer_steps, &cs_config, &cs_de_report);

	net_buf_simple_reset(&latest_local_steps);

	if (!(ras_feature_bits & RAS_FEAT_REALTIME_RD)) {
		net_buf_simple_reset(&latest_peer_steps);
	}

	k_sem_give(&sem_local_steps);

	cs_de_quality_t quality = cs_de_calc(&cs_de_report);

	if (quality == CS_DE_QUALITY_OK) {
		for (uint8_t ap = 0; ap < cs_de_report.n_ap; ap++) {
			if (cs_de_report.tone_quality[ap] == CS_DE_TONE_QUALITY_OK) {
				store_distance_estimates(&cs_de_report);
			}
		}
		k_sem_give(&sem_distance_estimate_updated);
	}
}

static void subevent_result_cb(struct bt_conn *conn, struct bt_conn_le_cs_subevent_result *result)
{
	if (dropped_ranging_counter == result->header.procedure_counter) {
		return;
	}

	if (most_recent_local_ranging_counter
		!= bt_ras_rreq_get_ranging_counter(result->header.procedure_counter)) {
		int sem_state = k_sem_take(&sem_local_steps, K_NO_WAIT);

		if (sem_state < 0) {
			dropped_ranging_counter = result->header.procedure_counter;
			LOG_INF("Dropped subevent results. Waiting for ranging data from peer.");
			return;
		}

		most_recent_local_ranging_counter =
			bt_ras_rreq_get_ranging_counter(result->header.procedure_counter);
	}

	if (result->header.subevent_done_status == BT_CONN_LE_CS_SUBEVENT_ABORTED) {
		/* The steps from this subevent will not be used. */
	} else if (result->step_data_buf) {
		if (result->step_data_buf->len <= net_buf_simple_tailroom(&latest_local_steps)) {
			uint16_t len = result->step_data_buf->len;
			uint8_t *step_data = net_buf_simple_pull_mem(result->step_data_buf, len);

			net_buf_simple_add_mem(&latest_local_steps, step_data, len);
		} else {
			LOG_ERR("Not enough memory to store step data. (%d > %d)",
				latest_local_steps.len + result->step_data_buf->len,
				latest_local_steps.size);
			net_buf_simple_reset(&latest_local_steps);
			dropped_ranging_counter = result->header.procedure_counter;
			return;
		}
	}

	dropped_ranging_counter = PROCEDURE_COUNTER_NONE;

	if (result->header.procedure_done_status == BT_CONN_LE_CS_PROCEDURE_COMPLETE) {
		most_recent_local_ranging_counter =
			bt_ras_rreq_get_ranging_counter(result->header.procedure_counter);
	} else if (result->header.procedure_done_status == BT_CONN_LE_CS_PROCEDURE_ABORTED) {
		LOG_WRN("Procedure %u aborted", result->header.procedure_counter);
		net_buf_simple_reset(&latest_local_steps);
		k_sem_give(&sem_local_steps);
	}
}

static void ranging_data_ready_cb(struct bt_conn *conn, uint16_t ranging_counter)
{
	LOG_DBG("Ranging data ready %i", ranging_counter);

	if (ranging_counter == most_recent_local_ranging_counter) {
		int err = bt_ras_rreq_cp_get_ranging_data(connection, &latest_peer_steps,
							  ranging_counter,
							  ranging_data_cb);
		if (err) {
			LOG_ERR("Get ranging data failed (err %d)", err);
			net_buf_simple_reset(&latest_local_steps);
			net_buf_simple_reset(&latest_peer_steps);
			k_sem_give(&sem_local_steps);
		}
	}
}

static void ranging_data_overwritten_cb(struct bt_conn *conn, uint16_t ranging_counter)
{
	LOG_INF("Ranging data overwritten %i", ranging_counter);
}

static void mtu_exchange_cb(struct bt_conn *conn, uint8_t err,
			    struct bt_gatt_exchange_params *params)
{
	if (err) {
		LOG_ERR("MTU exchange failed (err %d)", err);
		return;
	}

	LOG_INF("MTU exchange success (%u)", bt_gatt_get_mtu(conn));
	k_sem_give(&sem_mtu_exchange_done);
}

static void discovery_completed_cb(struct bt_gatt_dm *dm, void *context)
{
	int err;

	LOG_INF("The discovery procedure succeeded");

	struct bt_conn *conn = bt_gatt_dm_conn_get(dm);

	bt_gatt_dm_data_print(dm);

	err = bt_ras_rreq_alloc_and_assign_handles(dm, conn);
	if (err) {
		LOG_ERR("RAS RREQ alloc init failed (err %d)", err);
	}

	err = bt_gatt_dm_data_release(dm);
	if (err) {
		LOG_ERR("Could not release the discovery data (err %d)", err);
	}

	k_sem_give(&sem_discovery_done);
}

static void discovery_service_not_found_cb(struct bt_conn *conn, void *context)
{
	LOG_INF("The service could not be found during the discovery, disconnecting");
	bt_conn_disconnect(connection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

static void discovery_error_found_cb(struct bt_conn *conn, int err, void *context)
{
	LOG_INF("The discovery procedure failed (err %d)", err);
	bt_conn_disconnect(connection, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

static struct bt_gatt_dm_cb discovery_cb = {
	.completed = discovery_completed_cb,
	.service_not_found = discovery_service_not_found_cb,
	.error_found = discovery_error_found_cb,
};

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		LOG_ERR("Security failed: %s level %u err %d %s", addr, level, err,
			bt_security_err_to_str(err));
		return;
	}

	LOG_INF("Security changed: %s level %u", addr, level);
	k_sem_give(&sem_security);
}

static bool le_param_req(struct bt_conn *conn, struct bt_le_conn_param *param)
{
	/* Ignore peer parameter preferences. */
	return false;
}

static void connected_cb(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	(void)bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Connected to %s (err 0x%02X)", addr, err);

	if (err) {
		bt_conn_unref(conn);
		connection = NULL;
	} else {
		connection = bt_conn_ref(conn);

		k_sem_give(&sem_connected);

		dk_set_led_on(CON_STATUS_LED);
		bt_connected = true;  /* Enable distance-based LED color */
		led_pulse_step = 0;   /* Reset pulse animation */
	}
}

static void disconnected_cb(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason 0x%02X)", reason);

	bt_conn_unref(conn);
	connection = NULL;
	dk_set_led_off(CON_STATUS_LED);
	bt_connected = false; /* Switch to pulsing red LED */
	current_display_distance = 0.0f;

	sys_reboot(SYS_REBOOT_COLD);
}

static void remote_capabilities_cb(struct bt_conn *conn,
				   uint8_t status,
				   struct bt_conn_le_cs_capabilities *params)
{
	ARG_UNUSED(conn);
	ARG_UNUSED(params);

	if (status == BT_HCI_ERR_SUCCESS) {
		LOG_INF("CS capability exchange completed.");
		k_sem_give(&sem_remote_capabilities_obtained);
	} else {
		LOG_WRN("CS capability exchange failed. (HCI status 0x%02x)", status);
	}
}

static void config_create_cb(struct bt_conn *conn, uint8_t status,
			     struct bt_conn_le_cs_config *config)
{
	ARG_UNUSED(conn);

	if (status == BT_HCI_ERR_SUCCESS) {
		cs_config = *config;

		const char *mode_str[5] = {"Unused", "1 (RTT)", "2 (PBR)", "3 (RTT + PBR)",
					   "Invalid"};
		const char *role_str[3] = {"Initiator", "Reflector", "Invalid"};
		const char *rtt_type_str[8] = {
			"AA only",	 "32-bit sounding", "96-bit sounding", "32-bit random",
			"64-bit random", "96-bit random",   "128-bit random",  "Invalid"};
		const char *phy_str[4] = {"Invalid", "LE 1M PHY", "LE 2M PHY", "LE 2M 2BT PHY"};
		const char *chsel_type_str[3] = {"Algorithm #3b", "Algorithm #3c", "Invalid"};
		const char *ch3c_shape_str[3] = {"Hat shape", "X shape", "Invalid"};

		uint8_t mode_idx = config->mode > 0 && config->mode < 4 ? config->mode : 4;
		uint8_t role_idx = MIN(config->role, 2);
		uint8_t rtt_type_idx = MIN(config->rtt_type, 7);
		uint8_t phy_idx = config->cs_sync_phy > 0 && config->cs_sync_phy < 4
					  ? config->cs_sync_phy
					  : 0;
		uint8_t chsel_type_idx = MIN(config->channel_selection_type, 2);
		uint8_t ch3c_shape_idx = MIN(config->ch3c_shape, 2);

		LOG_INF("CS config creation complete.\n"
			" - id: %u\n"
			" - mode: %s\n"
			" - min_main_mode_steps: %u\n"
			" - max_main_mode_steps: %u\n"
			" - main_mode_repetition: %u\n"
			" - mode_0_steps: %u\n"
			" - role: %s\n"
			" - rtt_type: %s\n"
			" - cs_sync_phy: %s\n"
			" - channel_map_repetition: %u\n"
			" - channel_selection_type: %s\n"
			" - ch3c_shape: %s\n"
			" - ch3c_jump: %u\n"
			" - t_ip1_time_us: %u\n"
			" - t_ip2_time_us: %u\n"
			" - t_fcs_time_us: %u\n"
			" - t_pm_time_us: %u\n"
			" - channel_map: 0x%08X%08X%04X\n",
			config->id, mode_str[mode_idx],
			config->min_main_mode_steps, config->max_main_mode_steps,
			config->main_mode_repetition, config->mode_0_steps, role_str[role_idx],
			rtt_type_str[rtt_type_idx], phy_str[phy_idx],
			config->channel_map_repetition, chsel_type_str[chsel_type_idx],
			ch3c_shape_str[ch3c_shape_idx], config->ch3c_jump, config->t_ip1_time_us,
			config->t_ip2_time_us, config->t_fcs_time_us, config->t_pm_time_us,
			sys_get_le32(&config->channel_map[6]),
			sys_get_le32(&config->channel_map[2]),
			sys_get_le16(&config->channel_map[0]));

		k_sem_give(&sem_config_created);
	} else {
		LOG_WRN("CS config creation failed. (HCI status 0x%02x)", status);
	}
}

static void security_enable_cb(struct bt_conn *conn, uint8_t status)
{
	ARG_UNUSED(conn);

	if (status == BT_HCI_ERR_SUCCESS) {
		LOG_INF("CS security enabled.");
		k_sem_give(&sem_cs_security_enabled);
	} else {
		LOG_WRN("CS security enable failed. (HCI status 0x%02x)", status);
	}
}

static void procedure_enable_cb(struct bt_conn *conn,
				uint8_t status,
				struct bt_conn_le_cs_procedure_enable_complete *params)
{
	ARG_UNUSED(conn);

	if (status == BT_HCI_ERR_SUCCESS) {
		if (params->state == 1) {
			LOG_INF("CS procedures enabled:\n"
				" - config ID: %u\n"
				" - antenna configuration index: %u\n"
				" - TX power: %d dbm\n"
				" - subevent length: %u us\n"
				" - subevents per event: %u\n"
				" - subevent interval: %u\n"
				" - event interval: %u\n"
				" - procedure interval: %u\n"
				" - procedure count: %u\n"
				" - maximum procedure length: %u",
				params->config_id, params->tone_antenna_config_selection,
				params->selected_tx_power, params->subevent_len,
				params->subevents_per_event, params->subevent_interval,
				params->event_interval, params->procedure_interval,
				params->procedure_count, params->max_procedure_len);
		} else {
			LOG_INF("CS procedures disabled.");
		}
	} else {
		LOG_WRN("CS procedures enable failed. (HCI status 0x%02x)", status);
	}
}

void ras_features_read_cb(struct bt_conn *conn, uint32_t feature_bits, int err)
{
	if (err) {
		LOG_WRN("Error while reading RAS feature bits (err %d)", err);
	} else {
		LOG_INF("Read RAS feature bits: 0x%x", feature_bits);
		ras_feature_bits = feature_bits;
	}

	k_sem_give(&sem_ras_features);
}

static void scan_filter_match(struct bt_scan_device_info *device_info,
			      struct bt_scan_filter_match *filter_match, bool connectable)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));

	LOG_INF("Filters matched. Address: %s connectable: %d", addr, connectable);
}

static void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	int err;

	LOG_INF("Connecting failed, restarting scanning");

	err = bt_scan_start(BT_SCAN_TYPE_SCAN_PASSIVE);
	if (err) {
		LOG_ERR("Failed to restart scanning (err %i)", err);
		return;
	}
}

static void scan_connecting(struct bt_scan_device_info *device_info, struct bt_conn *conn)
{
	LOG_INF("Connecting");
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match, NULL, scan_connecting_error, scan_connecting);

static int scan_init(void)
{
	int err;

	struct bt_scan_init_param param = {
		.scan_param = NULL,
		.conn_param = BT_LE_CONN_PARAM(0x10, 0x10, 0, BT_GAP_MS_TO_CONN_TIMEOUT(4000)),
		.connect_if_match = 1};

	bt_scan_init(&param);
	bt_scan_cb_register(&scan_cb);

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_UUID, BT_UUID_RANGING_SERVICE);
	if (err) {
		LOG_ERR("Scanning filters cannot be set (err %d)", err);
		return err;
	}

	err = bt_scan_filter_enable(BT_SCAN_UUID_FILTER, false);
	if (err) {
		LOG_ERR("Filters cannot be turned on (err %d)", err);
		return err;
	}

	return 0;
}

BT_CONN_CB_DEFINE(conn_cb) = {
	.connected = connected_cb,
	.disconnected = disconnected_cb,
	.le_param_req = le_param_req,
	.security_changed = security_changed,
	.le_cs_read_remote_capabilities_complete = remote_capabilities_cb,
	.le_cs_config_complete = config_create_cb,
	.le_cs_security_enable_complete = security_enable_cb,
	.le_cs_procedure_enable_complete = procedure_enable_cb,
	.le_cs_subevent_data_available = subevent_result_cb,
};

int main(void)
{
	int err;

	LOG_INF("Starting Channel Sounding Distance LED Demo");

	dk_leds_init();

	/* Start LED strip update thread */
	k_thread_create(&led_thread_data, led_thread_stack,
			K_THREAD_STACK_SIZEOF(led_thread_stack),
			(k_thread_entry_t)led_update_thread,
			NULL, NULL, NULL,
			K_PRIO_COOP(7), 0, K_NO_WAIT);
	k_thread_name_set(&led_thread_data, "led_update");
	LOG_INF("LED strip thread started");

	/* Configure RF switch BEFORE Bluetooth initialization */
	const struct device *gpio2 = DEVICE_DT_GET(DT_NODELABEL(gpio2));
	if (device_is_ready(gpio2)) {
		gpio_pin_configure(gpio2, 3, GPIO_OUTPUT_ACTIVE);   /* rfsw-pwr = HIGH */
		gpio_pin_configure(gpio2, 5, GPIO_OUTPUT_INACTIVE); /* rfsw-ctl = LOW */
		LOG_INF("RF switch configured for onboard antenna");
	} else {
		LOG_WRN("GPIO2 not ready, RF switch not configured");
	}

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return 0;
	}

	err = scan_init();
	if (err) {
		LOG_ERR("Scan init failed (err %d)", err);
		return 0;
	}

	err = bt_scan_start(BT_SCAN_TYPE_SCAN_PASSIVE);
	if (err) {
		LOG_ERR("Scanning failed to start (err %i)", err);
		return 0;
	}

	k_sem_take(&sem_connected, K_FOREVER);

	err = bt_conn_set_security(connection, BT_SECURITY_L2);
	if (err) {
		LOG_ERR("Failed to encrypt connection (err %d)", err);
		return 0;
	}

	k_sem_take(&sem_security, K_FOREVER);

	static struct bt_gatt_exchange_params mtu_exchange_params = {.func = mtu_exchange_cb};

	bt_gatt_exchange_mtu(connection, &mtu_exchange_params);

	k_sem_take(&sem_mtu_exchange_done, K_FOREVER);

	err = bt_gatt_dm_start(connection, BT_UUID_RANGING_SERVICE, &discovery_cb, NULL);
	if (err) {
		LOG_ERR("Discovery failed (err %d)", err);
		return 0;
	}

	k_sem_take(&sem_discovery_done, K_FOREVER);

	const struct bt_le_cs_set_default_settings_param default_settings = {
		.enable_initiator_role = true,
		.enable_reflector_role = false,
		.cs_sync_antenna_selection = BT_LE_CS_ANTENNA_SELECTION_OPT_REPETITIVE,
		.max_tx_power = BT_HCI_OP_LE_CS_MAX_MAX_TX_POWER,
	};

	err = bt_le_cs_set_default_settings(connection, &default_settings);
	if (err) {
		LOG_ERR("Failed to configure default CS settings (err %d)", err);
		return 0;
	}

	err = bt_ras_rreq_read_features(connection, ras_features_read_cb);
	if (err) {
		LOG_ERR("Could not get RAS features from peer (err %d)", err);
		return 0;
	}

	k_sem_take(&sem_ras_features, K_FOREVER);

	const bool realtime_rd = ras_feature_bits & RAS_FEAT_REALTIME_RD;

	if (realtime_rd) {
		err = bt_ras_rreq_realtime_rd_subscribe(connection,
							&latest_peer_steps,
							ranging_data_cb);
		if (err) {
			LOG_ERR("RAS RREQ Real-time ranging data subscribe failed (err %d)", err);
			return 0;
		}
	} else {
		err = bt_ras_rreq_rd_overwritten_subscribe(connection, ranging_data_overwritten_cb);
		if (err) {
			LOG_ERR("RAS RREQ ranging data overwritten subscribe failed (err %d)", err);
			return 0;
		}

		err = bt_ras_rreq_rd_ready_subscribe(connection, ranging_data_ready_cb);
		if (err) {
			LOG_ERR("RAS RREQ ranging data ready subscribe failed (err %d)", err);
			return 0;
		}

		err = bt_ras_rreq_on_demand_rd_subscribe(connection);
		if (err) {
			LOG_ERR("RAS RREQ On-demand ranging data subscribe failed (err %d)", err);
			return 0;
		}

		err = bt_ras_rreq_cp_subscribe(connection);
		if (err) {
			LOG_ERR("RAS RREQ CP subscribe failed (err %d)", err);
			return 0;
		}
	}

	err = bt_le_cs_read_remote_supported_capabilities(connection);
	if (err) {
		LOG_ERR("Failed to exchange CS capabilities (err %d)", err);
		return 0;
	}

	k_sem_take(&sem_remote_capabilities_obtained, K_FOREVER);

	struct bt_le_cs_create_config_params config_params = {
		.id = CS_CONFIG_ID,
		.mode = BT_CONN_LE_CS_MAIN_MODE_2_SUB_MODE_1,
		.min_main_mode_steps = 2,
		.max_main_mode_steps = 5,
		.main_mode_repetition = 0,
		.mode_0_steps = NUM_MODE_0_STEPS,
		.role = BT_CONN_LE_CS_ROLE_INITIATOR,
		.rtt_type = BT_CONN_LE_CS_RTT_TYPE_AA_ONLY,
		.cs_sync_phy = BT_CONN_LE_CS_SYNC_1M_PHY,
		.channel_map_repetition = 1,
		.channel_selection_type = BT_CONN_LE_CS_CHSEL_TYPE_3B,
		.ch3c_shape = BT_CONN_LE_CS_CH3C_SHAPE_HAT,
		.ch3c_jump = 2,
	};

	bt_le_cs_set_valid_chmap_bits(config_params.channel_map);

	err = bt_le_cs_create_config(connection, &config_params,
				     BT_LE_CS_CREATE_CONFIG_CONTEXT_LOCAL_AND_REMOTE);
	if (err) {
		LOG_ERR("Failed to create CS config (err %d)", err);
		return 0;
	}

	k_sem_take(&sem_config_created, K_FOREVER);

	err = bt_le_cs_security_enable(connection);
	if (err) {
		LOG_ERR("Failed to start CS Security (err %d)", err);
		return 0;
	}

	k_sem_take(&sem_cs_security_enabled, K_FOREVER);

	const struct bt_le_cs_set_procedure_parameters_param procedure_params = {
		.config_id = CS_CONFIG_ID,
		.max_procedure_len = 1000,
		.min_procedure_interval = realtime_rd ? 5 : 10,
		.max_procedure_interval = realtime_rd ? 5 : 10,
		.max_procedure_count = 0,
		.min_subevent_len = 16000,
		.max_subevent_len = 16000,
		.tone_antenna_config_selection = BT_LE_CS_TONE_ANTENNA_CONFIGURATION_A1_B1,
		.phy = BT_LE_CS_PROCEDURE_PHY_2M,
		.tx_power_delta = 0x80,
		.preferred_peer_antenna = BT_LE_CS_PROCEDURE_PREFERRED_PEER_ANTENNA_1,
		.snr_control_initiator = BT_LE_CS_SNR_CONTROL_NOT_USED,
		.snr_control_reflector = BT_LE_CS_SNR_CONTROL_NOT_USED,
	};

	err = bt_le_cs_set_procedure_parameters(connection, &procedure_params);
	if (err) {
		LOG_ERR("Failed to set procedure parameters (err %d)", err);
		return 0;
	}

	struct bt_le_cs_procedure_enable_param params = {
		.config_id = CS_CONFIG_ID,
		.enable = 1,
	};

	err = bt_le_cs_procedure_enable(connection, &params);
	if (err) {
		LOG_ERR("Failed to enable CS procedures (err %d)", err);
		return 0;
	}

	LOG_INF("=== Channel Sounding with Weighted Average Fusion ===");
	LOG_INF("Calibrated distance shown for each method + weighted average");
	LOG_INF("Weights: Phase=50%%, RTT=25%%, IFFT=25%% (2:1:1 ratio)");
	LOG_INF("========================================================");

	while (true) {
		k_sem_take(&sem_distance_estimate_updated, K_FOREVER);
		if (buffer_num_valid != 0) {
			for (uint8_t ap = 0; ap < MAX_AP; ap++) {
				cs_de_dist_estimates_t distance_on_ap = get_distance(ap);
				uint32_t timestamp = k_uptime_get_32();

				/* Calculate weighted average, including valid zero values */
				float weighted_sum = 0.0f;
				float weight_sum = 0.0f;

				/* Use finite check instead of threshold - valid zeros are real measurements */
				if (isfinite(distance_on_ap.ifft) && distance_on_ap.ifft >= 0.0f) {
					weighted_sum += distance_on_ap.ifft * IFFT_WEIGHT;
					weight_sum += IFFT_WEIGHT;
				}
				if (isfinite(distance_on_ap.phase_slope) && distance_on_ap.phase_slope >= 0.0f) {
					weighted_sum += distance_on_ap.phase_slope * PHASE_WEIGHT;
					weight_sum += PHASE_WEIGHT;
				}
				if (isfinite(distance_on_ap.rtt) && distance_on_ap.rtt >= 0.0f) {
					weighted_sum += distance_on_ap.rtt * RTT_WEIGHT;
					weight_sum += RTT_WEIGHT;
				}

				float best_estimate = 0.0f;
				if (weight_sum > 0.01f) {
					best_estimate = weighted_sum / weight_sum;
				}

				/* Update LED strip display distance */
				current_display_distance = best_estimate;

				/* Log individual calibrated values and weighted average */
				LOG_INF("[AP%u] IFFT:%.2fm Phase:%.2fm RTT:%.2fm → Best:%.2fm",
					ap,
					(double)distance_on_ap.ifft,
					(double)distance_on_ap.phase_slope,
					(double)distance_on_ap.rtt,
					(double)best_estimate);
			}
		}
	}

	return 0;
}
