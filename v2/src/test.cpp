#include "core.h"
#include "test.h"

void debug_bytes(const byte_array &ba) {
	printf("[");
	for (int i = 0; i < (int)ba.size(); ++i) {
		printf("%u%c", (unsigned)((unsigned char)ba[i]), i == (int)ba.size() - 1 ? ']' : ' ');
	}
	printf("\n");
}

void debug_bytes_hex(const byte_array &ba) {
	printf("[");
	for (int i = 0; i < (int)ba.size(); ++i) {
		for (int j = 0; j < 2; ++j) {
			int d = (unsigned char)ba[i] >> (4 * (1 - j)) & 15;
			printf("%c", d < 10 ? '0' + d : 'a' + (d - 10));
		}
	}
	printf("]\n");
}

