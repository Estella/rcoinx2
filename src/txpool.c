#include "txpool.h"
#include "map.h"

void* transaction_pool;

void init_txpool() {
	transaction_pool = llqueue_new();
}

void txpool_add(struct tx* transaction) {
	llqueue_offer(transaction_pool, transaction);
}

struct tx* txpool_get() {
	return llqueue_poll(transaction_pool);
}
