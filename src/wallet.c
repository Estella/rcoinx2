#include <ed25519.h> // curvy primitives
#include "wallet.h"
#include "main.h"
#include "block.h"
#include <string.h>
FUNCTION void new_wallet(struct wallet* wallet) {
	char seed[32];
	ed25519_create_seed(seed);
	ed25519_create_keypair(wallet->public, wallet->private, seed);
}

FUNCTION void wallet_sign_transaction(uint8_t* private, struct tx* transaction) {
	memset(transaction->signature, '\0', 64);
	char public[32];
	ed25519_get_public(public, private);
	ed25519_sign(transaction->signature, (void*)&transaction->body, sizeof(struct tx_body), public, private);
}
FUNCTION int wallet_verify_transaction(uint8_t* public, struct tx *transaction) {
	return ed25519_verify(transaction->signature, (void*)&transaction->body, sizeof(struct tx_body), public);
}

FUNCTION int save_wallet(struct wallet* wallet, char *path) {
	char path2[256];
	if (path[0] != '/' && path[0] != '.') {
		snprintf(path2, 256, "%s/%s", get_options()->datadir, path);
	} else {
		snprintf(path2, 256, "%s", path);
	}
	FILE *f = fopen(path2, "wb");
	if (!f) return 0;
	fwrite(wallet, 1, sizeof(struct wallet), f);
	fclose(f);
	return 1;
}


FUNCTION int load_wallet(struct wallet* wallet, char *path) {
	char path2[256];
	if (path[0] != '/' && path[0] != '.') {
		snprintf(path2, 256, "%s/%s", get_options()->datadir, path);
	} else {
		snprintf(path2, 256, "%s", path);
	}
	FILE *f = fopen(path2, "rb");
	if (!f) return 0;
	fread(wallet, 1, sizeof(struct wallet), f);
	fclose(f);
	return 1;
}


