#include "block.h"
#include "debug.h"
#include "chain.h"
#include "tx.h"
#include "txpool.h"
int verify_transaction_standalone(struct tx* tx) {
	return wallet_verify_transaction(tx->body.from, tx);
}
int verify_transaction(struct tx* tx) {
	// verify transaction on the blockchain
	uint64_t balance = get_balance_for_address(tx->body.from);
	IF_DEBUG(printf("verifybalance() -> %lld\n", balance));
	if (balance < tx->body.amount) return 0;
	return verify_transaction_standalone(tx);
}
void broadcast_tx(struct tx* tx) {
	txpool_add(tx);
}
