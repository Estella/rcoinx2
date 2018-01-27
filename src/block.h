#pragma once
#pragma pack()
#include <stdint.h>
#define TX_PER_BLOCK 8
#define FLOATAMT(a) (((double)a)/100000.0)
#define INTAMT(a) (uint64_t)((a)*100000)
struct tx_body {
	uint64_t timestamp;
	uint8_t from[32];
	uint8_t to[32];
	uint64_t amount;
	uint8_t fee_percent;
	uint8_t message[32];
};
struct tx {
	uint8_t signature[64];
	struct tx_body body;
};
struct block {
	uint8_t hash[64];
	uint8_t lasthash[64];
	uint8_t num_tx;
	uint32_t nonce;
	uint64_t timestamp;
	struct tx transactions[TX_PER_BLOCK];
};
/* This is important */
static struct block genesis_block = {
	.hash = {0},
	.lasthash = {0},
	.num_tx = 1,
	.nonce = 0,
	.timestamp = 1517070928,
	.transactions = {
		{ {0}, {1517070928, {0}, {0}, 2500000000000, 0, {0}} }
	}
};
