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
	uint64_t padding_64;
};
FUNCTION void new_wallet(struct wallet* wallet);
FUNCTION void wallet_sign_transaction(struct wallet* wallet, struct tx* transaction);
FUNCTION int wallet_verify_transaction(uint8_t* public, struct tx *transaction);
FUNCTION int save_wallet(struct wallet* wallet, char *path);
FUNCTION int load_wallet(struct wallet* wallet, char *path);
