#include <ctime>
#include <string>
#include <cstring>
#include <fstream>

#include "core.h"
#include "util.h"
 
typedef unsigned char uint8;
typedef unsigned int uint32;
typedef unsigned long long uint64;
 
const unsigned int SHA224_256_BLOCK_SIZE = 512 / 8;
const unsigned int DIGEST_SIZE = 256 / 8;
 
unsigned int m_tot_len;
unsigned int m_len;
unsigned char m_block[2*SHA224_256_BLOCK_SIZE];
uint32 m_h[8];

void compress(uint32 state[8], const uint8 block[64]) {
	#define ROTR32(x, n)  (((0U + (x)) << (32 - (n))) | ((x) >> (n)))  // Assumes that x is uint32_t and 0 < n < 32
	
	#define LOADSCHEDULE(i)  \
		schedule[i] = (uint32)block[i * 4 + 0] << 24  \
		            | (uint32)block[i * 4 + 1] << 16  \
		            | (uint32)block[i * 4 + 2] <<  8  \
		            | (uint32)block[i * 4 + 3] <<  0;
	
	#define SCHEDULE(i)  \
		schedule[i] = 0U + schedule[i - 16] + schedule[i - 7]  \
			+ (ROTR32(schedule[i - 15], 7) ^ ROTR32(schedule[i - 15], 18) ^ (schedule[i - 15] >> 3))  \
			+ (ROTR32(schedule[i - 2], 17) ^ ROTR32(schedule[i - 2], 19) ^ (schedule[i - 2] >> 10));
	
	#define ROUND(a, b, c, d, e, f, g, h, i, k) \
		h = 0U + h + (ROTR32(e, 6) ^ ROTR32(e, 11) ^ ROTR32(e, 25)) + (g ^ (e & (f ^ g))) + UINT32_C(k) + schedule[i];  \
		d = 0U + d + h;  \
		h = 0U + h + (ROTR32(a, 2) ^ ROTR32(a, 13) ^ ROTR32(a, 22)) + ((a & (b | c)) | (b & c));
	
	static uint32 schedule[64];
	uint32 a = state[0];
	uint32 b = state[1];
	uint32 c = state[2];
	uint32 d = state[3];
	uint32 e = state[4];
	uint32 f = state[5];
	uint32 g = state[6];
	uint32 h = state[7];

	LOADSCHEDULE( 0)
	ROUND(a, b, c, d, e, f, g, h,  0, 0x428A2F98)
	LOADSCHEDULE( 1)
	ROUND(h, a, b, c, d, e, f, g,  1, 0x71374491)
	LOADSCHEDULE( 2)
	ROUND(g, h, a, b, c, d, e, f,  2, 0xB5C0FBCF)
	LOADSCHEDULE( 3)
	ROUND(f, g, h, a, b, c, d, e,  3, 0xE9B5DBA5)
	LOADSCHEDULE( 4)
	ROUND(e, f, g, h, a, b, c, d,  4, 0x3956C25B)
	LOADSCHEDULE( 5)
	ROUND(d, e, f, g, h, a, b, c,  5, 0x59F111F1)
	LOADSCHEDULE( 6)
	ROUND(c, d, e, f, g, h, a, b,  6, 0x923F82A4)
	LOADSCHEDULE( 7)
	ROUND(b, c, d, e, f, g, h, a,  7, 0xAB1C5ED5)
	LOADSCHEDULE( 8)
	ROUND(a, b, c, d, e, f, g, h,  8, 0xD807AA98)
	LOADSCHEDULE( 9)
	ROUND(h, a, b, c, d, e, f, g,  9, 0x12835B01)
	LOADSCHEDULE(10)
	ROUND(g, h, a, b, c, d, e, f, 10, 0x243185BE)
	LOADSCHEDULE(11)
	ROUND(f, g, h, a, b, c, d, e, 11, 0x550C7DC3)
	LOADSCHEDULE(12)
	ROUND(e, f, g, h, a, b, c, d, 12, 0x72BE5D74)
	LOADSCHEDULE(13)
	ROUND(d, e, f, g, h, a, b, c, 13, 0x80DEB1FE)
	LOADSCHEDULE(14)
	ROUND(c, d, e, f, g, h, a, b, 14, 0x9BDC06A7)
	LOADSCHEDULE(15)
	ROUND(b, c, d, e, f, g, h, a, 15, 0xC19BF174)
	SCHEDULE(16)
	ROUND(a, b, c, d, e, f, g, h, 16, 0xE49B69C1)
	SCHEDULE(17)
	ROUND(h, a, b, c, d, e, f, g, 17, 0xEFBE4786)
	SCHEDULE(18)
	ROUND(g, h, a, b, c, d, e, f, 18, 0x0FC19DC6)
	SCHEDULE(19)
	ROUND(f, g, h, a, b, c, d, e, 19, 0x240CA1CC)
	SCHEDULE(20)
	ROUND(e, f, g, h, a, b, c, d, 20, 0x2DE92C6F)
	SCHEDULE(21)
	ROUND(d, e, f, g, h, a, b, c, 21, 0x4A7484AA)
	SCHEDULE(22)
	ROUND(c, d, e, f, g, h, a, b, 22, 0x5CB0A9DC)
	SCHEDULE(23)
	ROUND(b, c, d, e, f, g, h, a, 23, 0x76F988DA)
	SCHEDULE(24)
	ROUND(a, b, c, d, e, f, g, h, 24, 0x983E5152)
	SCHEDULE(25)
	ROUND(h, a, b, c, d, e, f, g, 25, 0xA831C66D)
	SCHEDULE(26)
	ROUND(g, h, a, b, c, d, e, f, 26, 0xB00327C8)
	SCHEDULE(27)
	ROUND(f, g, h, a, b, c, d, e, 27, 0xBF597FC7)
	SCHEDULE(28)
	ROUND(e, f, g, h, a, b, c, d, 28, 0xC6E00BF3)
	SCHEDULE(29)
	ROUND(d, e, f, g, h, a, b, c, 29, 0xD5A79147)
	SCHEDULE(30)
	ROUND(c, d, e, f, g, h, a, b, 30, 0x06CA6351)
	SCHEDULE(31)
	ROUND(b, c, d, e, f, g, h, a, 31, 0x14292967)
	SCHEDULE(32)
	ROUND(a, b, c, d, e, f, g, h, 32, 0x27B70A85)
	SCHEDULE(33)
	ROUND(h, a, b, c, d, e, f, g, 33, 0x2E1B2138)
	SCHEDULE(34)
	ROUND(g, h, a, b, c, d, e, f, 34, 0x4D2C6DFC)
	SCHEDULE(35)
	ROUND(f, g, h, a, b, c, d, e, 35, 0x53380D13)
	SCHEDULE(36)
	ROUND(e, f, g, h, a, b, c, d, 36, 0x650A7354)
	SCHEDULE(37)
	ROUND(d, e, f, g, h, a, b, c, 37, 0x766A0ABB)
	SCHEDULE(38)
	ROUND(c, d, e, f, g, h, a, b, 38, 0x81C2C92E)
	SCHEDULE(39)
	ROUND(b, c, d, e, f, g, h, a, 39, 0x92722C85)
	SCHEDULE(40)
	ROUND(a, b, c, d, e, f, g, h, 40, 0xA2BFE8A1)
	SCHEDULE(41)
	ROUND(h, a, b, c, d, e, f, g, 41, 0xA81A664B)
	SCHEDULE(42)
	ROUND(g, h, a, b, c, d, e, f, 42, 0xC24B8B70)
	SCHEDULE(43)
	ROUND(f, g, h, a, b, c, d, e, 43, 0xC76C51A3)
	SCHEDULE(44)
	ROUND(e, f, g, h, a, b, c, d, 44, 0xD192E819)
	SCHEDULE(45)
	ROUND(d, e, f, g, h, a, b, c, 45, 0xD6990624)
	SCHEDULE(46)
	ROUND(c, d, e, f, g, h, a, b, 46, 0xF40E3585)
	SCHEDULE(47)
	ROUND(b, c, d, e, f, g, h, a, 47, 0x106AA070)
	SCHEDULE(48)
	ROUND(a, b, c, d, e, f, g, h, 48, 0x19A4C116)
	SCHEDULE(49)
	ROUND(h, a, b, c, d, e, f, g, 49, 0x1E376C08)
	SCHEDULE(50)
	ROUND(g, h, a, b, c, d, e, f, 50, 0x2748774C)
	SCHEDULE(51)
	ROUND(f, g, h, a, b, c, d, e, 51, 0x34B0BCB5)
	SCHEDULE(52)
	ROUND(e, f, g, h, a, b, c, d, 52, 0x391C0CB3)
	SCHEDULE(53)
	ROUND(d, e, f, g, h, a, b, c, 53, 0x4ED8AA4A)
	SCHEDULE(54)
	ROUND(c, d, e, f, g, h, a, b, 54, 0x5B9CCA4F)
	SCHEDULE(55)
	ROUND(b, c, d, e, f, g, h, a, 55, 0x682E6FF3)
	SCHEDULE(56)
	ROUND(a, b, c, d, e, f, g, h, 56, 0x748F82EE)
	SCHEDULE(57)
	ROUND(h, a, b, c, d, e, f, g, 57, 0x78A5636F)
	SCHEDULE(58)
	ROUND(g, h, a, b, c, d, e, f, 58, 0x84C87814)
	SCHEDULE(59)
	ROUND(f, g, h, a, b, c, d, e, 59, 0x8CC70208)
	SCHEDULE(60)
	ROUND(e, f, g, h, a, b, c, d, 60, 0x90BEFFFA)
	SCHEDULE(61)
	ROUND(d, e, f, g, h, a, b, c, 61, 0xA4506CEB)
	SCHEDULE(62)
	ROUND(c, d, e, f, g, h, a, b, 62, 0xBEF9A3F7)
	SCHEDULE(63)
	ROUND(b, c, d, e, f, g, h, a, 63, 0xC67178F2)
	
	state[0] = 0U + state[0] + a;
	state[1] = 0U + state[1] + b;
	state[2] = 0U + state[2] + c;
	state[3] = 0U + state[3] + d;
	state[4] = 0U + state[4] + e;
	state[5] = 0U + state[5] + f;
	state[6] = 0U + state[6] + g;
	state[7] = 0U + state[7] + h;
}

void transform(const unsigned char *message, unsigned int block_nb)
{
	for (int i = 0; i < block_nb; ++i) {
		compress(m_h, message + (i << 6));
	}
}
 
void init()
{
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
 
void update(const unsigned char *message, unsigned int len)
{
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
 
#define SHA2_UNPACK32(x, str)                 \
{                                             \
    *((str) + 3) = (uint8) ((x)      );       \
    *((str) + 2) = (uint8) ((x) >>  8);       \
    *((str) + 1) = (uint8) ((x) >> 16);       \
    *((str) + 0) = (uint8) ((x) >> 24);       \
}

void end(unsigned char *digest)
{
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
 
void sha256(unsigned char *digest, const char *input, const int length)
{
    memset(digest, 0, DIGEST_SIZE);
    init();
    update((unsigned char*)input, length);
    end(digest);
}

void mine(byte_array &rawb) {
	clock_t start = clock();
	int nonce = 0;
	const char *begin = rawb.data();
	unsigned char buf[32];
	bool flag = true;
	while (flag) {
		
		sha256(buf, begin + 32, rawb.size() - 32);

		flag = false;
		for (int i = 0; i < difficulty; ++i) {
			if (buf[i] != '0') {
				flag = true;
			}
		}

		if (flag) {
			inplace_replace(rawb, 32, 32, int_to_byte_array32(++nonce));
		}
	}
	inplace_replace(rawb, 0, 32, std::string((char *)buf, 32));
	if (verbose) {
		double sec = (double)(clock() - start) / CLOCKS_PER_SEC;
		printf("%d hashes %.6f seconds %.6f hashes/sec\n", nonce + 1, sec, (nonce + 1) / sec);
	}
}
