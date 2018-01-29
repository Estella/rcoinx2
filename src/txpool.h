#pragma once
#include "block.h"
extern void* transaction_pool;

void txpool_add(struct tx* transaction);
struct tx* txpool_get();
void txpool_remove(struct tx*);
int txpool_size();
