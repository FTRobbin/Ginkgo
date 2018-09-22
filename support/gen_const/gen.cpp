#include<string>
#include<cstdio>
#include<cstring>
#include<iostream>
#include<algorithm>

#include"sha256.h"

using namespace std;

const char* MY_NETID = "hn332";

typedef string byte_array;

typedef byte_array SHA256b;

byte_array int_to_byte_array(unsigned i) {
	byte_array ret = "";
	if (i == 0) {
		ret = "0";
	} else {
		while (i) {
			ret += (unsigned char)('0' + (i % 10));
			i /= 10;
		}
		reverse(ret.begin(), ret.end());
	}
	return ret;
}

byte_array string_to_byte_array(string s) {
	return s;
}

inline char hexify(int d) {
	return d < 10 ? '0' + d : 'a' + (d - 10);
}

void pretty_print(string s) {
	printf("\"");
	for (int i = 0; i < (int)s.size(); ++i) {
		char high = hexify((unsigned char)s[i] >> 4), low = hexify(s[i] & 15);
		printf("\\x%c%c", high, low);
	}
	printf("\"");
}

void gen_UTXO() {
	printf("const char* ACCOUNT_ADDS[100] = {\n");
	for (int i = 0; i < 100; ++i) {
		SHA256b account = sha256(int_to_byte_array(i));
		pretty_print(account);
		if (i < 100 - 1) {
			printf(",\n");
		} else {
			printf("};\n\n");
		}
	}
}

void gen_netid() {
	SHA256b netid = sha256(string_to_byte_array(MY_NETID));
	printf("const char* MINER_ADD = ");
	pretty_print(netid);
	printf(";\n");
}

void gen_gehash() {
	SHA256b gehash = sha256(int_to_byte_array(0));
	printf("const char* HASH0 = ");
	pretty_print(gehash);
	printf(";\n");
}

int main() {
	printf("#ifndef ADDR_h\n");
	printf("#define ADDR_h\n\n");
	gen_UTXO();
	gen_netid();
	gen_gehash();
	printf("\n#endif\n");
	return 0;
}
