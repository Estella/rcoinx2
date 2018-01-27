#pragma once
#include <stdint.h>
#include "block.h"
#include "debug.h"
#pragma pack()
struct wallet {
	uint8_t private[64];
	uint8_t public[32];
	uint64_t amount;
};
struct remote_wallet {
	uint8_t padding[64];
	uint8_t public[32];
	uint64_t padding;
};
