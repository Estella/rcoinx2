#include <ed25519.h> // curvy primitives
#include "wallet.h"

void new_wallet(struct wallet* wallet) {
	char seed[32];
	ed25519_create_seed(seed);
	ed25519_create_keypair(wallet->public, wallet->private, seed);
}

void wallet_sign_transaction(struct wallet* wallet, struct tx* transaction) {
	memset(transaction->signature, '\0', 64);
	ed25519_sign(transaction->signature, transaction->body, sizeof(struct tx_body), wallet->public, wallet->private);
}
void wallet_verify_transaction(uint8_t* public, struct tx *transaction) {
	return ed25519_verify(transaction->signature, transaction->body, sizeof(struct tx_body), public);
}
