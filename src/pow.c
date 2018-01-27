#include <sha512.h>
#include <aes.h>
#include "block.h"
#include "debug.h"
#include "pow.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bn.h"
#define BOX_X 256
#define BOX_Y 256
FUNCTION uint32_t calc_target(uint32_t last_target, uint64_t time_a, uint64_t time_b) {
	int32_t difference = (time_a - time_b);
	if (difference > TARGET_TIME_SEC && last_target == 0) return 1;
	return last_target + ((TARGET_TIME_SEC * 100) - (difference * 100));
}
FUNCTION int verify_work(uint8_t *hash, uint32_t target) {
	struct bn a;
	struct bn c;
	struct bn d;
	struct bn b;
	memset(a.array, 0xFF, 64);
	bignum_from_int(&c, target);
	bignum_div(&a, &c, &d);
	memcpy(b.array, hash, 64);
	IF_DEBUG(
		/*char hexhash[512];
		bignum_to_string(&b, hexhash, 512);
		printf("Hash: %s\n", hexhash);*/
	)
	return bignum_cmp(&d, &b) == LARGER;
}
// TODO: multithreading
FUNCTION void proof_of_work(struct block* orig_block, int *cancel_flag, uint32_t target, int *hashespersec) {
	memset(orig_block->hash, 0, 64);
	orig_block->nonce = 1;
	int hashes = 0; *hashespersec = 0;
	clock_t clockstart = clock();
	char tmp[64];
	while (!cancel_flag || !*cancel_flag) {
		if (orig_block->nonce == 0) orig_block->timestamp++;
		hash(orig_block, tmp);
		hashes++;
		if (verify_work(tmp, target)) {
			IF_DEBUG(printf("Completed block\n"));
			break;
		}
		if ((clock()/CLOCKS_PER_SEC) != (clockstart/CLOCKS_PER_SEC)) {
			*hashespersec = hashes;
			hashes = 0;
			clockstart = clock();
		IF_DEBUG(
			printf("%d H/s\n", *hashespersec);
		)
		}
		orig_block->nonce++;
	}
	memcpy(orig_block->hash, tmp, 64);
	return;
}
FUNCTION void hash(struct block *orig_block, uint8_t* out) {
	struct block block = *orig_block;
	sha512_context shactx;
	struct AES_ctx ctx;
	char tmp[64];
	char box[BOX_X*BOX_Y];
	sha512_init(&shactx);
	sha512_update(&shactx, (void*)&block, sizeof(struct block));
	sha512_final(&shactx, tmp);
	for (int y = 0; y < BOX_Y; y++)
		for (int x = 0; x < BOX_X; x++)
			box[(y * BOX_X) + x] = tmp[(x^y)&0x40];
	AES_init_ctx_iv(&ctx, tmp, tmp);
	AES_CTR_xcrypt_buffer(&ctx, (void*)&block, sizeof(struct block));
	AES_CTR_xcrypt_buffer(&ctx, box, sizeof(box));
	sha512_init(&shactx);
	sha512_update(&shactx, tmp, 64);
	sha512_update(&shactx, box, sizeof(box));
	sha512_update(&shactx, (void*)&block, sizeof(struct block));
	sha512_final(&shactx, tmp);
	memcpy(out, tmp, 64);
}
