#include "txpool.h"
#include "queue.h"
#include <string.h>
void* transaction_pool;
int txpool_size_ = 0;
void init_txpool() {
	transaction_pool = llqueue_new();
}

void txpool_add(struct tx* transaction) {
	txpool_size_++;
	llqueue_offer(transaction_pool, transaction);
}

struct tx* txpool_get() {
	txpool_size_--;
	return llqueue_poll(transaction_pool);
}

int txpool_size() { return txpool_size_; }

void txpool_remove(struct tx* tx) {
	llqueue_remove_item_via_cmpfunction(transaction_pool, tx, memcmp);
}
