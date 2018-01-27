#include <stdio.h>
#include "block.h"
#include "pow.h"

int main() {
	struct block block; int hps = 0;
	proof_of_work(&block, NULL, 0x2, &hps);
}
