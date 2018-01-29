#include "tinycthread.h"
#include "yhs.h"
#include "rpc.h"
#include <ed25519.h>
#include <string.h>
#include "tx.h"
#include "base32.h"
#include "debug.h"
#include "block.h"
#include "chain.h"
#include "mining.h"
#include "wallet.h"
#include "main.h"
#include <stdio.h>
#include "txpool.h"
#include <time.h>
#include "easyjsonencode.h"
#define enforce_local() if(!strstr(yhs_get_ip(req), "127.")&&!get_options()->allow_all) return
void rpc_gettemplate(yhsRequest *req) {
	enforce_local();
	yhs_begin_data_response(req, "application/binary");
	struct block b;
	get_block_template(&b);
	yhs_data(req, &b, sizeof(struct block));
}
void get_block_template(struct block* o) {
	struct block a;
	read_block(1, get_height() - 1, &a);
	struct block b;
	memset(&b, 0, sizeof(struct block));
	b.version = 0x01;
	memcpy(b.lasthash, a.hash, 64);
	b.timestamp = time(NULL);
	b.num_tx = 1;
	b.transactions[0] = COINBASE_TX;
	struct tx* tx; int i = 0;
	for (i = 0, tx = txpool_get(); i < TX_PER_BLOCK && tx; i++, tx = txpool_get()) {
		b.transactions[i+1] = *tx;
	}
	b.num_tx += i;
	memcpy(o, &b, sizeof(struct block));
}
void rpc_status(yhsRequest* req) {
	enforce_local();
	yhs_begin_data_response(req, "application/json");
	char data[2048];
	easy_json_encode(data, 2048, JSON_STR, "status", "offline", JSON_INT, "difficulty", get_difficulty(1), JSON_UINT64, "height", get_height(), JSON_UINT64, "reward", calc_reward(get_height()), JSON_INT, "txpool_size", txpool_size(), 0);
	yhs_text(req, data);
}
void rpc_newwallet(yhsRequest *req) {
	enforce_local();
	yhs_begin_data_response(req, "application/json");
	struct wallet w;
	char basebuf[256]; char basebuf2[256];
	new_wallet(&w);
	base32_encode(w.private, 64, basebuf, 256);
	base32_encode(w.public, 32, basebuf2, 256);
	char data[2048];
	easy_json_encode(data, 2048, JSON_STR, "private", basebuf, JSON_STR, "public", basebuf2, 0);
	yhs_text(req, data);
}
void rpc_getbalance(yhsRequest *req) {
	enforce_local();
	if (!yhs_read_form_content(req)) return;
	char *addr = yhs_find_control_value(req, "address");
	if (!addr) return;
	yhs_begin_data_response(req, "application/json");
	char data[1024]; char pub[32];
	base32_decode(addr, pub, 32);
	easy_json_encode(data, 1024, JSON_UINT64, "balance", get_balance_for_address(pub), 0);
	yhs_text(req, data);
}
void rpc_walletsend(yhsRequest *req) {
	enforce_local();
	if (!yhs_read_form_content(req)) return;
	const char *priv = yhs_find_control_value(req, "key");
	const char *tostr = yhs_find_control_value(req, "to");
	const char *amtstr = yhs_find_control_value(req, "amount");
	const char *feestr = yhs_find_control_value(req, "fee");
	if (!priv || !amtstr || !tostr) return;
	uint64_t amt = atoll(amtstr);
	uint16_t fee = atoll(feestr ? feestr : "500");
	char privkey[64]; char to[32]; char out[512];
	char hash[256]; char pubkey[32];
	base32_decode(priv, privkey, 64);
	ed25519_get_public(pubkey, privkey);
	base32_decode(tostr, to, 32);
	struct tx* tx = calloc(1, sizeof(struct tx));
	memcpy(tx->body.from, pubkey, 32);
	memcpy(tx->body.to, to, 32);
	tx->body.amount = fee + amt;
	tx->body.fee = fee;
	tx->body.nonce = rand() % 65536;
	tx->body.timestamp = time(NULL);
	wallet_sign_transaction(privkey, tx);
	yhs_begin_data_response(req, "application/json");
	if (!verify_transaction(tx))
		{ easy_json_encode(out, 512, JSON_STR, "status", "failed", 0);
		yhs_text(req, out); return; }
	broadcast_tx(tx);
	base32_encode(tx->signature, 64, hash, 256);
	easy_json_encode(out, 512, JSON_STR, "status", "ok", JSON_STR, "id", hash, 0);
	yhs_text(req, out);
}
uint8_t payto[32];
int mine_hps;
int mine_stop = 1;
int mine_skip = 0;
int mining_height;
void rpc_miningstatus(yhsRequest *req) {
	enforce_local();
	yhs_begin_data_response(req, "application/json");
	char buf[1024];
	easy_json_encode(buf, 1024, JSON_OBJ, "mining", mine_stop ? "false" : "true", JSON_INT, "hashrate", mine_hps, 0);
	yhs_text(req, buf);
}
int mining_start_thread(void* UNUSED) {
	start_mining(payto, NULL, &mine_skip, &mine_stop, &mine_hps);
}
int mining_monitor_thread(void* UNUSED) {
	mining_height = get_height();
	thrd_t t;
	thrd_create(&t, mining_start_thread, NULL);
	while (!mine_stop) {
		if (get_height() > mining_height) {
			mine_skip = 1; mining_height = get_height();
		}
		thrd_yield();
	}
}
void rpc_miningstart(yhsRequest *req) {
	enforce_local();
	if (!yhs_read_form_content(req)) return;
	const char *addr = yhs_find_control_value(req, "address");
	if (!addr) return;
	base32_decode(addr, payto, 32);
	yhs_begin_data_response(req, "application/json");
	if (mine_stop) {
		thrd_t t;
		mining_height = get_height();
		mine_stop = 0;
		thrd_create(&t, mining_monitor_thread, NULL);
	}
	yhs_text(req, "{\"success\": true}");
}
FUNCTION void init_rpc_server(int port) {
	log_info("now listening on rpc port %d...", port);

	yhsServer *s = yhs_new_server(port);
	yhs_add_res_path_handler(s, "/status", &rpc_status, NULL);
	yhs_add_res_path_handler(s, "/mining/status", &rpc_miningstatus, NULL);
	yhs_set_valid_methods(YHS_METHOD_POST, yhs_add_res_path_handler(s, "/getbalance", &rpc_getbalance, NULL));
	yhs_set_valid_methods(YHS_METHOD_POST, yhs_add_res_path_handler(s, "/mining/start", &rpc_miningstart, NULL));
	yhs_set_valid_methods(YHS_METHOD_POST, yhs_add_res_path_handler(s, "/wallet/tx", &rpc_walletsend, NULL));
	yhs_add_res_path_handler(s, "/wallet/new", &rpc_newwallet, NULL);
	yhs_add_res_path_handler(s, "/getblocktemplate", &rpc_gettemplate, NULL);
	while (1) yhs_update(s);
}
/*struct Response* createResponseForRequest(const struct Request* request, struct Connection* connection) {
	if (!strcmp(request->pathDecoded, "/status")) {
		char data[2048];
		easy_json_encode(data, 2048, JSON_STR, "status", "offline", JSON_INT, "difficulty", get_difficulty(1), JSON_UINT64, "height", get_height(), 0);
		return responseAllocJSON(data);
	}
	if (!strcmp(request->pathDecoded, "/getblocktemplate")) {
		struct block a;
		read_block(1, get_height() - 1, &a);
		struct block b;
		memset(&b, 0, sizeof(struct block));
		b.version = 0x01;
		memcpy(b.lasthash, a.hash, 64);
		b.timestamp = time(NULL);
		// TODO pull transactions from pool
		struct Response* r = responseAlloc(200, "OK", "application/x-rcoinx-block-template", sizeof(struct block));
		memcpy(r->body.contents, &b, sizeof(struct block));
		r->body.length = sizeof(struct block);
		return r;
	}
	if (!strcmp(request->pathDecoded, "/balance")) {
	}
	if (!strcmp(request->pathDecoded, "/wallet/new")) {
		struct wallet w;
		char basebuf[256]; char basebuf2[256];
		new_wallet(&w);
		base32_encode(w.private, 64, basebuf, 256);
		base32_encode(w.public, 32, basebuf2, 256);
		char data[2048];
		easy_json_encode(data, 2048, JSON_STR, "private", basebuf, JSON_STR, "public", basebuf2, 0);
		return responseAllocJSON(data);
	}
	if (!strcmp(request->pathDecoded, "/wallet/send")) {
		struct wallet w;
		char *private = strdupDecodeGETParam("private=", request, "");
		if (!private[0]) {
			free(private);
			return responseAllocJSON(FAILURE_STRING);
		}
		base32_decode(private, w.private, 64);
		
	}
	return responseAlloc404NotFoundHTML("{ \"failed\": 404 }");
}
*/
