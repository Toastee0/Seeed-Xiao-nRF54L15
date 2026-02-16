/*
 * FindMy Beacon - nRF54L15 Port
 * Based on macless-haystack nRF52 firmware
 * 
 * This implements Apple's FindMy protocol for tracking devices
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>
#include <string.h>
#include "openhaystack.h"

LOG_MODULE_REGISTER(findmy, LOG_LEVEL_INF);

#define ADVERTISING_INTERVAL_MS     5000   // 5 seconds between advertisements
#define KEY_ROTATION_ENABLED        0      // 0 = disabled (saves power), 1 = enabled
#define KEY_ROTATION_INTERVAL_MIN   30     // Rotate key every 30 minutes (if enabled)
#define BATTERY_UPDATE_INTERVAL_DAYS 14    // Update battery status every 14 days
#define MAX_KEYS                    20     // Maximum number of keys

// Convert intervals to milliseconds
#define KEY_ROTATION_INTERVAL_MS (KEY_ROTATION_INTERVAL_MIN * 60 * 1000)
#define BATTERY_UPDATE_INTERVAL_MS (BATTERY_UPDATE_INTERVAL_DAYS * 24 * 60 * 60 * 1000)

// Key storage - initialize with placeholder
static char public_keys[MAX_KEYS][28] = {
    {0x07, 0xC2, 0x17, 0x57, 0xD7, 0xCD, 0xA2, 0xB2, 0xE9, 0x51,
     0x20, 0xCB, 0x0E, 0xA4, 0x8B, 0x36, 0x16, 0xB2, 0x97, 0x1C,
     0x8E, 0x23, 0x90, 0x23, 0x3A, 0x5C, 0x80, 0xC9},
};

static int last_filled_index = -1;
static int current_key_index = 0;
static uint8_t adv_data[31];
static uint8_t adv_data_len = 0;
static uint8_t identity_id = 0;  // Store the identity ID

// Bluetooth address derived from current key
static bt_addr_le_t current_addr;

// Advertising parameters - non-connectable beacon
static struct bt_le_adv_param adv_param = {
    .id = 0,  // Will be set dynamically
    .options = (BT_LE_ADV_OPT_USE_IDENTITY),  // Non-connectable by not setting CONNECTABLE
    .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,  // 100ms
    .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,  // 150ms
    .peer = NULL,
};

// Work items for timers
static struct k_work_delayable key_rotation_work;
static struct k_work_delayable battery_update_work;

/*
 * Update battery level in advertisement data
 * For now, uses placeholder - integrate with ADC later
 */
static void update_battery_level(void)
{
    // TODO: Implement actual battery reading via ADC
    // For now, set battery status byte to 0x00 (full battery)
    if (adv_data_len > 4) {
        adv_data[4] = 0x00;  // State byte (bit 0-1: battery level)
    }
    LOG_INF("Battery level updated");
}

/*
 * Set MAC address from the first 6 bytes of the public key
 */
static void set_mac_from_key(const char *key)
{
    current_addr.type = BT_ADDR_LE_RANDOM;
    current_addr.a.val[5] = key[0] | 0xC0;  // Mark as random static address
    current_addr.a.val[4] = key[1];
    current_addr.a.val[3] = key[2];
    current_addr.a.val[2] = key[3];
    current_addr.a.val[1] = key[4];
    current_addr.a.val[0] = key[5];
}

/*
 * Prepare advertisement data from public key
 */
static void prepare_adv_data(const char *key)
{
    adv_data_len = set_advertisement_key(key, adv_data);
    update_battery_level();
}

/*
 * Start advertising with current key
 */
static int start_advertising(void)
{
    int err;
    struct bt_data ad[] = {
        BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
        BT_DATA(BT_DATA_NAME_COMPLETE, "FindMy", 6),
        BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_data, adv_data_len),
    };

    // Stop any existing advertising
    bt_le_adv_stop();

    // Update advertising parameters with current identity
    adv_param.id = identity_id;

    LOG_INF("Starting advertising on identity %d with %d bytes", identity_id, adv_data_len);

    // Start advertising
    err = bt_le_adv_start(&adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return err;
    }

    LOG_INF("Advertising started (key index: %d)", current_key_index);
    return 0;
}

/*
 * Rotate to next key and restart advertising
 */
static void set_and_advertise_next_key(void)
{
    int new_id;
    
    // Stop current advertising
    bt_le_adv_stop();

    // Move to next key (wrap around)
    current_key_index = (current_key_index + 1) % (last_filled_index + 1);

    // Set MAC address from new key
    set_mac_from_key(public_keys[current_key_index]);

    // Prepare advertisement data
    prepare_adv_data(public_keys[current_key_index]);

    // Delete old identity and create new one with new MAC
    bt_id_delete(identity_id);
    new_id = bt_id_create(&current_addr, NULL);
    if (new_id < 0) {
        LOG_ERR("Failed to create new identity (err %d)", new_id);
        return;
    }
    identity_id = new_id;

    // Start advertising with new key
    start_advertising();

    LOG_INF("Rotated to key %d", current_key_index);
}

/*
 * Key rotation timer handler
 */
static void key_rotation_handler(struct k_work *work)
{
    set_and_advertise_next_key();

    // Reschedule for next rotation
    k_work_schedule(&key_rotation_work, K_MSEC(KEY_ROTATION_INTERVAL_MS));
}

/*
 * Battery update timer handler
 */
static void battery_update_handler(struct k_work *work)
{
    update_battery_level();

    // Update advertisement data without changing key
    int err = bt_le_adv_update_data(
        (struct bt_data[]) {
            BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_data, adv_data_len),
        }, 1, NULL, 0);

    if (err) {
        LOG_ERR("Failed to update advertisement data (err %d)", err);
    }

    // Reschedule for next battery update
    k_work_schedule(&battery_update_work, K_MSEC(BATTERY_UPDATE_INTERVAL_MS));
}

/*
 * Main entry point
 */
int main(void)
{
    int err;

    LOG_INF("FindMy Beacon starting...");

    // Find last filled key index
    for (int i = MAX_KEYS - 1; i >= 0; i--) {
        if (strlen(public_keys[i]) > 0) {
            last_filled_index = i;
            break;
        }
    }

    if (last_filled_index < 0) {
        LOG_ERR("No public keys configured!");
        return -1;
    }

    LOG_INF("Found %d key(s) to advertise", last_filled_index + 1);

    // Set up first key MAC and advertisement data
    set_mac_from_key(public_keys[0]);
    prepare_adv_data(public_keys[0]);

    LOG_INF("Target MAC: %02X:%02X:%02X:%02X:%02X:%02X", 
            current_addr.a.val[5], current_addr.a.val[4], current_addr.a.val[3],
            current_addr.a.val[2], current_addr.a.val[1], current_addr.a.val[0]);

    // Create identity with the MAC from first key BEFORE bt_enable
    identity_id = bt_id_create(&current_addr, NULL);
    if (identity_id < 0) {
        LOG_ERR("Failed to create identity (err %d)", identity_id);
        return identity_id;
    }
    LOG_INF("Created identity %d", identity_id);

    // Initialize Bluetooth
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return err;
    }

    LOG_INF("Bluetooth initialized");

    // Initialize work items
    k_work_init_delayable(&key_rotation_work, key_rotation_handler);
    k_work_init_delayable(&battery_update_work, battery_update_handler);

    // Start advertising
    err = start_advertising();
    if (err) {
        return err;
    }

#if KEY_ROTATION_ENABLED
    // Schedule key rotation only if enabled and we have multiple keys
    if (last_filled_index > 0) {
        k_work_schedule(&key_rotation_work, K_MSEC(KEY_ROTATION_INTERVAL_MS));
        LOG_INF("Key rotation enabled: every %d minutes", KEY_ROTATION_INTERVAL_MIN);
    }
#else
    LOG_INF("Key rotation disabled (power saving mode)");
#endif

    // Schedule battery updates
    k_work_schedule(&battery_update_work, K_MSEC(BATTERY_UPDATE_INTERVAL_MS));
    LOG_INF("Battery updates scheduled every %d days", BATTERY_UPDATE_INTERVAL_DAYS);

    LOG_INF("FindMy Beacon running");

    // Main loop - Zephyr handles power management automatically
    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
