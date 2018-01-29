#pragma once
#pragma pack()
#include <stdint.h>
#include <stdio.h>
#include "block_def.h"
#include "debug.h"
#include "chain.h"
#include "base32.h"
#define FLOATAMT(a) (((double)a)/100000.0)
#define INTAMT(a) (uint64_t)((a)*100000)
#define COINBASE_TX ((struct tx){ \
		{0}, {time(NULL), {0}, {0}, calc_reward(get_height()), \
			0, "Coinbase transaction", 0}})
/* This is important */
IF_DEBUG(
	static void print_block(struct block *b) {
		char printstr[512];
		int l = sprintf(printstr, "Block hash: ");
		l += base32_encode(b->hash, 64, printstr + l, 128);
		l += sprintf(printstr + l, "\nLast hash: ");
		l += base32_encode(b->lasthash, 64, printstr + l, 128);
		l += sprintf(printstr + l, "\nNum. TXs (including coinbase): %d\n", b->num_tx);
		puts(printstr);
	}
)
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
#include "pow.h"
#include "chain.h"
#include "main.h"
static inline int verify_block(struct block *b, int mainchain) {
	if (!verify_work(b->hash, get_difficulty(mainchain))) { IF_DEBUG(printf("Failed to verify work\n")); return 0; }
	struct block ob;
	IF_DEBUG(printf("Block at %d\n", get_height() - 1));
	read_block(1, get_height() - 1, &ob);
	IF_DEBUG(print_block(&ob));
	if (memcmp(b->lasthash, ob.hash, 64)) { IF_DEBUG(printf("Block chain out of order\n")); return 0; }
	return b->transactions[0].body.amount <= calc_reward(mainchain ? get_height() : __get_alt_height());
}
