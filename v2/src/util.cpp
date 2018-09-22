#include <cassert>
#include <cstring>
#include <string>

#include "core.h"
#include "util.h"

using namespace std;

bool overflow_10_9(const byte_array &ba) {
	assert(ba.size() == 32);
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

int byte_array_to_int(const byte_array &ba) {
	int ret = 0;
	for (int i = 0; i < ba.length(); ++i) {
		ret = ret * 10 + (ba[i] - '0');
	}
	return ret;
}

byte_array int_to_byte_array32(int n) {
	char buf[32];
	for (int i = 0; i < 32; ++i) {
		buf[i] = '0';
	}
	int i = 32 - 1;
	while (n) {
		buf[i] += n % 10;
		n /= 10;
		--i;
	}
	return byte_array(buf, 32);
}

void inplace_replace(string &dest, int pos, int len, const string &source) {
	for (int i = 0; i < len; ++i) {
		dest[pos + i] = source[i];
	}
}
