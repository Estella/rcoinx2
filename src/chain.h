#pragma once
#include <stdint.h>
#include "block.h"
#include "miniz.h"
void init_blockchain();
uint64_t get_height();
uint64_t __get_alt_height();
void read_block(int, uint64_t, struct block*);
void write_block(int, uint64_t, struct block*);
uint64_t get_balance_for_address(uint8_t* public);
#define block_compress(in, out, outlen) (mz_compress((void*)out, (mz_ulong*)&outlen, (void*)in, 65536) == MZ_OK)
#define block_decompress(in, inlen, out) (__BLOCK_SIZE__ = 65536, mz_uncompress((void*)out, (mz_ulong*)&__BLOCK_SIZE__, (void*)in, inlen) == MZ_OK)
static volatile int __BLOCK_SIZE__ = sizeof(struct block);
