/*
 * OpenHaystack Protocol Implementation
 * Generates FindMy advertisement packets from public keys
 */

#ifndef OPENHAYSTACK_H
#define OPENHAYSTACK_H

#include <stdint.h>

/**
 * Set advertisement data from public key
 *
 * @param key       28-byte public key (base64 decoded)
 * @param data_out  Buffer to store advertisement data (min 31 bytes)
 * @return          Length of advertisement data (always 31)
 */
uint8_t set_advertisement_key(const char *key, uint8_t *data_out);

#endif /* OPENHAYSTACK_H */
