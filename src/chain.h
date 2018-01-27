#pragma once
#include <stdint.h>
#include "block.h"
void init_blockchain();
uint64_t get_height();
uint64_t __get_alt_height();
void read_block(int, uint64_t, struct block*);
void write_block(int, uint64_t, struct block*);
