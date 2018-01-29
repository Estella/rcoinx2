#pragma once
#include "debug.h"
#include "log.h"
#include "block.h"
FUNCTION void init_rpc_server(int port);
FUNCTION void get_block_template(struct block* out);
