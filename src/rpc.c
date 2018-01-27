#define EWS_HEADER_ONLY
#include "ews.h"
#include "rpc.h"
#include "base32.h"
#include "debug.h"
#include "block.h"
#include "chain.h"
#include "wallet.h"
#include "main.h"
#include <stdio.h>
#include <time.h>
#include "easyjsonencode.h"
FUNCTION void init_rpc_server(int port) {
	printf("listening on rpc port %d...\n", port);
	acceptConnectionsUntilStoppedFromEverywhereIPv4(NULL, port);
}
struct Response* createResponseForRequest(const struct Request* request, struct Connection* connection) {
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
	return responseAlloc404NotFoundHTML("{ \"failed\": 404 }");
}
