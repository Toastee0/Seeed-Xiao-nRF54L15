/*
 * OpenHaystack Protocol Implementation
 * Based on macless-haystack nRF52 firmware
 */

#include <stdint.h>
#include <string.h>
#include "openhaystack.h"

// FindMy advertisement template (Apple's offline finding protocol)
// NOTE: Length and type bytes are NOT included - will be added by BT stack
static const uint8_t offline_finding_adv_template[29] = {
    0x4c, 0x00, // Company ID (Apple)
    0x12, 0x19, // Offline Finding type and length
    0x00,       // State (will be updated with battery status)
    // 22 bytes of public key (bytes 6-27 of the key)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,       // First two bits of byte 0 from key
    0x00,       // Hint (0x00)
};

/**
 * Set advertisement data from public key
 *
 * The FindMy protocol uses:
 * - Bytes 0-5 of key: BLE MAC address (with 0xC0 mask on byte 0)
 * - Bytes 6-27 of key: Advertisement payload (22 bytes)
 * - Top 2 bits of byte 0: Appended to end of advertisement
 *
 * @param key       28-byte public key
 * @param data_out  Buffer to store advertisement data (must be >= 29 bytes)
 * @return          Length of advertisement data (29 bytes, BT stack adds length/type)
 */
uint8_t set_advertisement_key(const char *key, uint8_t *data_out)
{
    // Copy template (without length/type bytes)
    memcpy(data_out, offline_finding_adv_template, 29);

    // Insert bytes 6-27 of key into advertisement (22 bytes)
    memcpy(&data_out[5], &key[6], 22);

    // Append first two bits of byte 0 from key
    data_out[27] = key[0] >> 6;

    return 29;
}
