#include "yhs.h"
#include "rpc.h"
#include <string.h>
#include "base32.h"
#include "debug.h"
#include "block.h"
#include "chain.h"
#include "wallet.h"
#include "main.h"
#include <stdio.h>
#include <time.h>
#include "easyjsonencode.h"
#define enforce_local() if(!strstr(yhs_get_ip(req), "127.")&&!get_options()->allow_all) return
void rpc_gettemplate(yhsRequest *req) {
	enforce_local();
	yhs_begin_data_response(req, "application/binary");
	struct block a;
	read_block(1, get_height() - 1, &a);
	struct block b;
	memset(&b, 0, sizeof(struct block));
	b.version = 0x01;
	memcpy(b.lasthash, a.hash, 64);
	b.timestamp = time(NULL);
	// TODO pull transactions from pool
	yhs_data(req, &b, sizeof(struct block));
}
void rpc_status(yhsRequest* req) {
	enforce_local();
	yhs_begin_data_response(req, "application/json");
	char data[2048];
	easy_json_encode(data, 2048, JSON_STR, "status", "offline", JSON_INT, "difficulty", get_difficulty(1), JSON_UINT64, "height", get_height(), 0);
	yhs_textf(req, "%s", data);
}
FUNCTION void init_rpc_server(int port) {
	printf("now listening on rpc port %d...\n", port);
	yhsServer *s = yhs_new_server(port);
	yhs_add_res_path_handler(s, "/status", &rpc_status, NULL);
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
		char data[2048]; char outbuf[32];
		char *addr = strdupDecodeGETParam("address=", request, "LTE5JRYOCLSZKZ2XL6ZXGBNLI3CTN537FS2PAO7IMUV6WT7N4QRQ");
		base32_decode(addr, outbuf, 32);
		easy_json_encode(data, 2048, JSON_FLOAT, "balance", FLOATAMT(get_balance_for_address(outbuf)), 0);
		free(addr);
		return responseAllocJSON(data);
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
