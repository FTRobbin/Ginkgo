#include <ctime>
#include <cstdio>
#include <cstring>

#include <pthread.h>

#include <vector>
#include <algorithm>

#include "miner.h"

#include "network.h"

#include "util.h"

#include "core.h"

using namespace std;
 
typedef unsigned char uint8;
typedef unsigned int uint32;
 
const unsigned int SHA224_256_BLOCK_SIZE = 512 / 8;
const unsigned int DIGEST_SIZE = 256 / 8;

unsigned int m_tot_len;
unsigned int m_len;
unsigned char m_block[2*SHA224_256_BLOCK_SIZE];
uint32 m_h[8];

inline void compress(uint32 state[8], const uint8 block[64]) {
#define ROTR32(x, n)  (((x) << (32 - (n))) | ((x) >> (n)))  // Assumes that x is uint32_t and 0 < n < 32

#define LOADSCHEDULE(wi, i)  \
	wi = (uint32)block[i * 4 + 0] << 24  \
	| (uint32)block[i * 4 + 1] << 16  \
	| (uint32)block[i * 4 + 2] <<  8  \
	| (uint32)block[i * 4 + 3];

#define SCHEDULE(wi, wi7, wi15, wi2)  \
	wi += wi7  \
	+ (ROTR32(wi15, 7) ^ ROTR32(wi15, 18) ^ (wi15 >> 3))  \
	+ (ROTR32(wi2, 17) ^ ROTR32(wi2, 19) ^ (wi2 >> 10));

#define ROUND(a, b, c, d, e, f, g, h, wi, k) \
	h += (ROTR32(e, 6) ^ ROTR32(e, 11) ^ ROTR32(e, 25)) + (g ^ (e & (f ^ g))) + wi;  \
	d += h;  \
	h += (ROTR32(a, 2) ^ ROTR32(a, 13) ^ ROTR32(a, 22)) + ((a & (b | c)) | (b & c));

	uint32 w0, w1, w2 , w3 , w4 , w5 , w6 , w7,
		   w8, w9, w10, w11, w12, w13, w14, w15;
	uint32 a = state[0];
	uint32 b = state[1];
	uint32 c = state[2];
	uint32 d = state[3];
	uint32 e = state[4];
	uint32 f = state[5];
	uint32 g = state[6];
	uint32 h = state[7];

	LOADSCHEDULE(w0,0);
	ROUND(a,b,c,d,e,f,g,h,w0,0x428a2f98);
	LOADSCHEDULE(w1,1);
	ROUND(h,a,b,c,d,e,f,g,w1,0x71374491);
	LOADSCHEDULE(w2,2);
	ROUND(g,h,a,b,c,d,e,f,w2,0xb5c0fbcf);
	LOADSCHEDULE(w3,3);
	ROUND(f,g,h,a,b,c,d,e,w3,0xe9b5dba5);
	LOADSCHEDULE(w4,4);
	ROUND(e,f,g,h,a,b,c,d,w4,0x3956c25b);
	LOADSCHEDULE(w5,5);
	ROUND(d,e,f,g,h,a,b,c,w5,0x59f111f1);
	LOADSCHEDULE(w6,6);
	ROUND(c,d,e,f,g,h,a,b,w6,0x923f82a4);
	LOADSCHEDULE(w7,7);
	ROUND(b,c,d,e,f,g,h,a,w7,0xab1c5ed5);
	LOADSCHEDULE(w8,8);
	ROUND(a,b,c,d,e,f,g,h,w8,0xd807aa98);
	LOADSCHEDULE(w9,9);
	ROUND(h,a,b,c,d,e,f,g,w9,0x12835b01);
	LOADSCHEDULE(w10,10);
	ROUND(g,h,a,b,c,d,e,f,w10,0x243185be);
	LOADSCHEDULE(w11,11);
	ROUND(f,g,h,a,b,c,d,e,w11,0x550c7dc3);
	LOADSCHEDULE(w12,12);
	ROUND(e,f,g,h,a,b,c,d,w12,0x72be5d74);
	LOADSCHEDULE(w13,13);
	ROUND(d,e,f,g,h,a,b,c,w13,0x80deb1fe);
	LOADSCHEDULE(w14,14);
	ROUND(c,d,e,f,g,h,a,b,w14,0x9bdc06a7);
	LOADSCHEDULE(w15,15);
	ROUND(b,c,d,e,f,g,h,a,w15,0xc19bf174);
	SCHEDULE(w0,w9,w1,w14);
	ROUND(a,b,c,d,e,f,g,h,w0,0xe49b69c1);
	SCHEDULE(w1,w10,w2,w15);
	ROUND(h,a,b,c,d,e,f,g,w1,0xefbe4786);
	SCHEDULE(w2,w11,w3,w0);
	ROUND(g,h,a,b,c,d,e,f,w2,0xfc19dc6);
	SCHEDULE(w3,w12,w4,w1);
	ROUND(f,g,h,a,b,c,d,e,w3,0x240ca1cc);
	SCHEDULE(w4,w13,w5,w2);
	ROUND(e,f,g,h,a,b,c,d,w4,0x2de92c6f);
	SCHEDULE(w5,w14,w6,w3);
	ROUND(d,e,f,g,h,a,b,c,w5,0x4a7484aa);
	SCHEDULE(w6,w15,w7,w4);
	ROUND(c,d,e,f,g,h,a,b,w6,0x5cb0a9dc);
	SCHEDULE(w7,w0,w8,w5);
	ROUND(b,c,d,e,f,g,h,a,w7,0x76f988da);
	SCHEDULE(w8,w1,w9,w6);
	ROUND(a,b,c,d,e,f,g,h,w8,0x983e5152);
	SCHEDULE(w9,w2,w10,w7);
	ROUND(h,a,b,c,d,e,f,g,w9,0xa831c66d);
	SCHEDULE(w10,w3,w11,w8);
	ROUND(g,h,a,b,c,d,e,f,w10,0xb00327c8);
	SCHEDULE(w11,w4,w12,w9);
	ROUND(f,g,h,a,b,c,d,e,w11,0xbf597fc7);
	SCHEDULE(w12,w5,w13,w10);
	ROUND(e,f,g,h,a,b,c,d,w12,0xc6e00bf3);
	SCHEDULE(w13,w6,w14,w11);
	ROUND(d,e,f,g,h,a,b,c,w13,0xd5a79147);
	SCHEDULE(w14,w7,w15,w12);
	ROUND(c,d,e,f,g,h,a,b,w14,0x6ca6351);
	SCHEDULE(w15,w8,w0,w13);
	ROUND(b,c,d,e,f,g,h,a,w15,0x14292967);
	SCHEDULE(w0,w9,w1,w14);
	ROUND(a,b,c,d,e,f,g,h,w0,0x27b70a85);
	SCHEDULE(w1,w10,w2,w15);
	ROUND(h,a,b,c,d,e,f,g,w1,0x2e1b2138);
	SCHEDULE(w2,w11,w3,w0);
	ROUND(g,h,a,b,c,d,e,f,w2,0x4d2c6dfc);
	SCHEDULE(w3,w12,w4,w1);
	ROUND(f,g,h,a,b,c,d,e,w3,0x53380d13);
	SCHEDULE(w4,w13,w5,w2);
	ROUND(e,f,g,h,a,b,c,d,w4,0x650a7354);
	SCHEDULE(w5,w14,w6,w3);
	ROUND(d,e,f,g,h,a,b,c,w5,0x766a0abb);
	SCHEDULE(w6,w15,w7,w4);
	ROUND(c,d,e,f,g,h,a,b,w6,0x81c2c92e);
	SCHEDULE(w7,w0,w8,w5);
	ROUND(b,c,d,e,f,g,h,a,w7,0x92722c85);
	SCHEDULE(w8,w1,w9,w6);
	ROUND(a,b,c,d,e,f,g,h,w8,0xa2bfe8a1);
	SCHEDULE(w9,w2,w10,w7);
	ROUND(h,a,b,c,d,e,f,g,w9,0xa81a664b);
	SCHEDULE(w10,w3,w11,w8);
	ROUND(g,h,a,b,c,d,e,f,w10,0xc24b8b70);
	SCHEDULE(w11,w4,w12,w9);
	ROUND(f,g,h,a,b,c,d,e,w11,0xc76c51a3);
	SCHEDULE(w12,w5,w13,w10);
	ROUND(e,f,g,h,a,b,c,d,w12,0xd192e819);
	SCHEDULE(w13,w6,w14,w11);
	ROUND(d,e,f,g,h,a,b,c,w13,0xd6990624);
	SCHEDULE(w14,w7,w15,w12);
	ROUND(c,d,e,f,g,h,a,b,w14,0xf40e3585);
	SCHEDULE(w15,w8,w0,w13);
	ROUND(b,c,d,e,f,g,h,a,w15,0x106aa070);
	SCHEDULE(w0,w9,w1,w14);
	ROUND(a,b,c,d,e,f,g,h,w0,0x19a4c116);
	SCHEDULE(w1,w10,w2,w15);
	ROUND(h,a,b,c,d,e,f,g,w1,0x1e376c08);
	SCHEDULE(w2,w11,w3,w0);
	ROUND(g,h,a,b,c,d,e,f,w2,0x2748774c);
	SCHEDULE(w3,w12,w4,w1);
	ROUND(f,g,h,a,b,c,d,e,w3,0x34b0bcb5);
	SCHEDULE(w4,w13,w5,w2);
	ROUND(e,f,g,h,a,b,c,d,w4,0x391c0cb3);
	SCHEDULE(w5,w14,w6,w3);
	ROUND(d,e,f,g,h,a,b,c,w5,0x4ed8aa4a);
	SCHEDULE(w6,w15,w7,w4);
	ROUND(c,d,e,f,g,h,a,b,w6,0x5b9cca4f);
	SCHEDULE(w7,w0,w8,w5);
	ROUND(b,c,d,e,f,g,h,a,w7,0x682e6ff3);
	SCHEDULE(w8,w1,w9,w6);
	ROUND(a,b,c,d,e,f,g,h,w8,0x748f82ee);
	SCHEDULE(w9,w2,w10,w7);
	ROUND(h,a,b,c,d,e,f,g,w9,0x78a5636f);
	SCHEDULE(w10,w3,w11,w8);
	ROUND(g,h,a,b,c,d,e,f,w10,0x84c87814);
	SCHEDULE(w11,w4,w12,w9);
	ROUND(f,g,h,a,b,c,d,e,w11,0x8cc70208);
	SCHEDULE(w12,w5,w13,w10);
	ROUND(e,f,g,h,a,b,c,d,w12,0x90befffa);
	SCHEDULE(w13,w6,w14,w11);
	ROUND(d,e,f,g,h,a,b,c,w13,0xa4506ceb);
	SCHEDULE(w14,w7,w15,w12);
	ROUND(c,d,e,f,g,h,a,b,w14,0xbef9a3f7);
	SCHEDULE(w15,w8,w0,w13);
	ROUND(b,c,d,e,f,g,h,a,w15,0xc67178f2);

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	state[5] += f;
	state[6] += g;
	state[7] += h;
}

void transform(const unsigned char *message, unsigned int block_nb) {
	for (int i = 0; i < block_nb; ++i) {
		compress(m_h, message + (i << 6));
	}
}

#define SHA2_UNPACK32(x, str) {               \
	*((str) + 3) = (uint8) ((x)      );       \
	*((str) + 2) = (uint8) ((x) >>  8);       \
	*((str) + 1) = (uint8) ((x) >> 16);       \
	*((str) + 0) = (uint8) ((x) >> 24);       \
}

void init() {
	m_h[0] = 0x6a09e667;
	m_h[1] = 0xbb67ae85;
	m_h[2] = 0x3c6ef372;
	m_h[3] = 0xa54ff53a;
	m_h[4] = 0x510e527f;
	m_h[5] = 0x9b05688c;
	m_h[6] = 0x1f83d9ab;
	m_h[7] = 0x5be0cd19;
	m_len = 0;
	m_tot_len = 0;
}

void update(const unsigned char *message, unsigned int len) {
	unsigned int block_nb;
	unsigned int new_len, rem_len, tmp_len;
	const unsigned char *shifted_message;
	tmp_len = SHA224_256_BLOCK_SIZE - m_len;
	rem_len = len < tmp_len ? len : tmp_len;
	memcpy(&m_block[m_len], message, rem_len);
	if (m_len + len < SHA224_256_BLOCK_SIZE) {
		m_len += len;
		return;
	}
	new_len = len - rem_len;
	block_nb = new_len / SHA224_256_BLOCK_SIZE;
	shifted_message = message + rem_len;
	transform(m_block, 1);
	transform(shifted_message, block_nb);
	rem_len = new_len % SHA224_256_BLOCK_SIZE;
	memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
	m_len = rem_len;
	m_tot_len += (block_nb + 1) << 6;
}

void end(unsigned char *digest) {
	unsigned int block_nb;
	unsigned int pm_len;
	unsigned int len_b;
	int i;
	block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9)
				< (m_len % SHA224_256_BLOCK_SIZE)));
	len_b = (m_tot_len + m_len) << 3;
	pm_len = block_nb << 6;
	memset(m_block + m_len, 0, pm_len - m_len);
	m_block[m_len] = 0x80;
	SHA2_UNPACK32(len_b, m_block + pm_len - 4);
	transform(m_block, block_nb);
	for (i = 0 ; i < 8; i++) {
		SHA2_UNPACK32(m_h[i], &digest[i << 2]);
	}
}

void sha256(unsigned char *digest, const char *input, const int length) {
	memset(digest, 0, DIGEST_SIZE);
	init();
	update((unsigned char*)input, length);
	end(digest);
}

void mine(char *rawb) {
	clock_t start = clock();
	unsigned char buf[32];
	int nonce = 0;
	bool flag = true;
	while (flag) {

		sha256(buf, rawb + 32, block_size - 32);

		flag = false;
		for (int i = 0; i < difficulty; ++i) {
			if (buf[i] != '0') {
				flag = true;
			}
		}

		if (flag) {
			int_to_byte_array32(++nonce, rawb + 32);
		}
	}
	memcpy(rawb, buf, sizeof(char) * 32);
	if (verbose) {
		double sec = (double)(clock() - start) / CLOCKS_PER_SEC;
		printf("%d hashes %.6f seconds %.6f hashes/sec\n", nonce + 1, sec, (nonce + 1) / sec);
	}
}

//rewind operations

int rewind_len = 8;
int rewind_cnt;

vector<int> ord;

void transform_ord(const unsigned char *message) {
	for (int u = 0; u < rewind_len; ++u) {
		int i = ord[u] * 2;
		compress(m_h, message + (i << 6));
		compress(m_h, message + ((i | 1) << 6));
	}
}

unsigned int block_nb;
unsigned int new_len, rem_len, tmp_len;
const unsigned char *shifted_message;

void update_halt(const unsigned char *message, unsigned int len) {
	tmp_len = SHA224_256_BLOCK_SIZE - m_len;
	rem_len = len < tmp_len ? len : tmp_len;
	memcpy(&m_block[m_len], message, rem_len);
	if (m_len + len < SHA224_256_BLOCK_SIZE) {
		m_len += len;
		return;
	}
	new_len = len - rem_len;
	block_nb = new_len / SHA224_256_BLOCK_SIZE;
	shifted_message = message + rem_len;
	transform(m_block, 1);
	transform(shifted_message, block_nb - rewind_len * 2);
}

unsigned int bak_m_tot_len;
unsigned int bak_m_len;
unsigned char bak_m_block[2*SHA224_256_BLOCK_SIZE];
uint32 bak_m_h[8];

unsigned int bak_rem_len;

void record_state() {
	bak_m_tot_len = m_tot_len;
	bak_m_len = m_len;
	memcpy(bak_m_block, m_block, sizeof(m_block));
	memcpy(bak_m_h, m_h, sizeof(m_h));

	bak_rem_len = rem_len;
}

void rewind_state() {
	m_tot_len = bak_m_tot_len;
	m_len = bak_m_len;
	memcpy(m_block, bak_m_block, sizeof(m_block));
	memcpy(m_h, bak_m_h, sizeof(m_h));

	rem_len = bak_rem_len;
}

void update_cont() {
	transform_ord(shifted_message + ((block_nb - rewind_len * 2) << 6));
	rem_len = new_len % SHA224_256_BLOCK_SIZE;
	memcpy(m_block, &shifted_message[block_nb << 6], rem_len);
	m_len = rem_len;
	m_tot_len += (block_nb + 1) << 6;
}

void rewind_sha256(unsigned char *digest, const char *input, const int length) {
	rewind_cnt = 1;
	init();
	update_halt((unsigned char*)input, length);
	record_state();
	ord.resize(rewind_len);
	for (int i = 0; i < rewind_len; ++i) {
		ord[i] = i;
	}
	bool flag = true;
	while (flag) {
		update_cont();
		end(digest);
		flag = false;
		for (int i = 0; i < difficulty; ++i) {
			if (digest[i] != '0') {
				flag = true;
			}
		}
		if (flag) {
			if (!next_permutation(ord.begin(), ord.end())) {
				break;
			} else {
				rewind_state();
				++rewind_cnt;
			}
		}
	}
}

void rewind_mine(char *rawb) {
	clock_t start = clock();
	static unsigned char buf[32];
	int nonce = 0;
	int total_rewind = 0;
	bool flag = true;
	while (flag) {

		rewind_sha256(buf, rawb + 32, block_size - 32);

		total_rewind += rewind_cnt;

		flag = false;
		for (int i = 0; i < difficulty; ++i) {
			if (buf[i] != '0') {
				flag = true;
			}
		}

		if (flag) {
			int_to_byte_array32(++nonce, rawb + 32);
		}
	}
	memcpy(rawb, buf, sizeof(char) * 32);
	//Fixed to 128!!
	static char tmp[128 + 1];
	for (int i = 0; i < rewind_len; ++i) {
		if (i != ord[i]) {	
			char *txi = rawb + (block_size - (rewind_len - i) * tx_size),
				 *txord = rawb + (block_size - (rewind_len - ord[i]) * tx_size);

			memcpy(tmp, txi, tx_size);
			memcpy(txi, txord, tx_size);
			memcpy(txord, tmp, tx_size);

			for (int j = 0; j < rewind_len; ++j) {
				if (ord[j] == i) {
					ord[j] = ord[i];
					ord[i] = i;
					break;
				}
			}
		}
	}
	if (verbose) {
		double sec = (double)(clock() - start) / CLOCKS_PER_SEC;
		printf("%d hashes %d rewinds %.6f seconds %.6f rewinds/sec\n", nonce + 1, total_rewind, sec, total_rewind / sec);
	}
}

int miner_ptr = 0;

pthread_mutex_t miner_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t has_unmined = PTHREAD_COND_INITIALIZER;

bool miner_closing = false;

void *miner_main(void *ptr) {
	while (!miner_closing) {
		pthread_mutex_lock(&miner_lock);
		while (miner_ptr == chain.size()) {
			pthread_cond_wait(&has_unmined, &miner_lock);
		}
		pthread_mutex_unlock(&miner_lock);
		if (chain[miner_ptr] == NULL) { // stop signal
			miner_closing = true;
			broadcast_queue.push(NULL); // tell network_out to stop
			continue;
		}
		char *raw = chain[miner_ptr] + 1;
		if (use_rewind) {
			rewind_mine(raw);
		} else {
			mine(raw);
		}
		rotate(raw, raw + 32, raw + 96);
		++miner_ptr;
		broadcast_queue.push(raw - 1);
	}
}
