#include "mining.h"
#include "rpc.h"
#include "log.h"
#include "pow.h"
#include "chain.h"
void start_mining(uint8_t* payto, int *mined_blocks, int *skip_flag, int *stop_flag, int *hps) {
	while (!*stop_flag) {
		*skip_flag = 0;
		struct block b;
		get_block_template(&b);
		IF_DEBUG(print_block(&b));
		memcpy(b.transactions[0].body.to, payto, 32);
		proof_of_work(&b, skip_flag, get_difficulty(1), hps);
		if (*skip_flag) continue;
		if (verify_block(&b, 1)) {
			write_block(1, get_height(), &b);
			//TODO broadcast_block(&b);
		} else {
			log_warn("Mined an invalid block(?!?!)");
		}
	}
}
