#pragma once
#include "debug.h"
FUNCTION void proof_of_work(struct block* orig_block, int *cancel_flag, uint32_t target, int *hashspersec);
FUNCTION void hash(struct block* orig_block, uint8_t* out);
FUNCTION int verify_work(uint8_t* hash, uint32_t target);
FUNCTION int calc_target(uint64_t, uint64_t);

#define TARGET_TIME_SEC 30
