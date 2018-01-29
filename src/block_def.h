#pragma once
#include <stdint.h>
#define TX_PER_BLOCK 363
struct tx_body {
	uint64_t timestamp;
	uint8_t from[32];
	uint8_t to[32];
	uint64_t amount;
	uint16_t fee;
	uint8_t message[29];
	uint16_t nonce;
};
struct tx {
	uint8_t signature[64];
	struct tx_body body;
};
struct block {
	uint8_t version;
	uint8_t hash[64];
	uint8_t lasthash[64];
	uint8_t num_tx;
	uint32_t nonce;
	uint64_t timestamp;
	uint8_t reserved[51];
	struct tx transactions[TX_PER_BLOCK];
};
