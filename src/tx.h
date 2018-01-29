#pragma once
#include "block.h"
int verify_transaction_standalone(struct tx*);
int verify_transaction(struct tx*);
void broadcast_tx(struct tx*);
