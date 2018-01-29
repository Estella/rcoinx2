#pragma once
#pragma pack()
#include <stdint.h>
#define TX_PER_BLOCK 363
#define FLOATAMT(a) (((double)a)/100000.0)
#define INTAMT(a) (uint64_t)((a)*100000)
struct tx_body {
	uint64_t timestamp;
	uint8_t from[32];
	uint8_t to[32];
	uint64_t amount;
	uint8_t fee_percent;
	uint8_t message[30];
	uint16_t nonce;
};
struct tx {
	uint8_t signature[64];
	struct tx_body body;
};
#define COINBASE_TX ((struct tx){ \
		{0}, {time(NULL), {0}, {0}, calc_reward(get_height()), \
			0, "Coinbase transaction", 0}})
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
/* This is important */
#include "base32.h"
static inline char* __GENESIS_BLOCK__() {
	static char buf[32];
	base32_decode("LTE5JRYOCLSZKZ2XL6ZXGBNLI3CTN537FS2PAO7IMUV6WT7N4QRQ", buf, 32);
	return buf;
}
static struct block genesis_block = {
	.hash = {0},
	.lasthash = {0},
	.num_tx = 1,
	.nonce = 0,
	.timestamp = 1517070928,
	.transactions = {
		{ {0}, {1517070928, {0}, {0}, 2500000000000ULL, 0, "XRX BLOCK 0", 0} }
	}
};
static inline uint64_t calc_reward(uint64_t height) {
	uint64_t START_REWARD = 250 * 100000;
	for (uint64_t i = 0; i < (height/3153600); i++) {
		if (START_REWARD < 100000) return 100000;
		START_REWARD /= 2;
	}
	return START_REWARD;
}
