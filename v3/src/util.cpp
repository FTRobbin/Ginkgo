#include <cstring>

#include "util.h"

using namespace std;

bool overflow_10_9(const char *ba) {
	int length = 10;
	for (int i = 0; i < 32 - length; ++i) {
		if (ba[i] != '0') {
			return true;
		}
	}
	if (ba[32 - length] < '0' || ba[32 - length] > '1') {
		return true;
	}
	for (int i = 32 - length + 1; i < 32; ++i) {
		if (ba[i] < '0' || ba[i] > '9') {
			return true;
		}
	}
	return false;
}

int byte_array32_to_int(const char *ba) {
	int ret = 0;
	for (int i = 0; i < 32; ++i) {
		ret = ret * 10 + (ba[i] - '0');
	}
	return ret;
}

long long byte_array_to_long_long(const char *ba, const int length) {
	long long ret = 0;
	for (int i = 0; i < length; ++i) {
		ret = ret * 10 + (ba[i] - '0');
	}
	return ret;
}

void int_to_byte_array32(int n, char *buf) {
	memset(buf, '0', sizeof(char) * 32);
	int i = 32 - 1;
	while (n) {
		buf[i] += n % 10;
		n /= 10;
		--i;
	}
}
