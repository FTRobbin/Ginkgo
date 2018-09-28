#include <cstring>
#include <ctime>

#include <vector>
#include <stdexcept>
#include <algorithm>

#include <ext/pb_ds/assoc_container.hpp>

#include "addr.h"
#include "core.h"
#include "network.h"
#include "miner.h"
#include "util.h"

using namespace std;

const int INIT_AMOUNT = 100000;

int member[256 + 1][2];

int UTXO[100];

void init_UTXO() {
	memset(member, -1, sizeof(member));
	for (int i = 0; i < 100; ++i) {
		unsigned head = (unsigned char)ACCOUNT_ADDS[i][0];
		if (member[head][0] == -1) {
			member[head][0] = i;
		} else if (member[head][1] == -1) {
			member[head][1] = i;
		} else {
			fprintf(stderr, "Unexpected hash\n");
			throw runtime_error("");
		}
		UTXO[i] = INIT_AMOUNT;
	}
}

int get_id(const char *hash) {
	unsigned head = (unsigned char)hash[0];
	for (int i = 0; i < 2; ++i) {
		int j = member[head][i];
		if (j != -1 && memcmp(hash, ACCOUNT_ADDS[j], 32) == 0) {
			return j;
		}
	}
	return -1;
}

int get_sender_id(const char *tx) {
	return get_id(tx);
}

int get_receiver_id(const char *tx) {
	return get_id(tx + 32);
}

int get_amount(const char *tx) {
	const char *amt = tx + 64;
	if (overflow_10_9(amt)) {
		return -1;
	} else {
		return byte_array32_to_int(amt);
	}
}

long long get_timestamp(const char *tx) {
	const char *ts = tx + 96;
	long long timestamp_h = byte_array_to_long_long(ts, 14),
			  timestamp_l = byte_array_to_long_long(ts + 14, 18);
	if (timestamp_h != 0 && verbose) {
		fprintf(stderr, "Warning: extra large time stamp");
	}
	return timestamp_l;
}

int get_block_height(const char *m) {
	const char *he = m + 1;
	if (overflow_10_9(he)) {
		return -1;
	} else {
		return byte_array32_to_int(he);
	}
}

//consider this potentially dangerous
__gnu_pbds::gp_hash_table<long long, __gnu_pbds::null_type> history[100][100];

const int HASH_A = 10007;

long long hasher(const int &amount, const long long &timestamp) {
	return timestamp * HASH_A + amount;
}

bool is_duplicate_tx(const int &sender, const int &receiver, const int &amount, const long long &timestamp) {
	return history[sender][receiver].find(hasher(amount, timestamp)) != history[sender][receiver].end();
}

void insert_tx(const int &sender, const int &receiver, const int &amount, const long long &timestamp) {
	history[sender][receiver].insert(hasher(amount, timestamp));
}

char *current_block;

int num_tx;

vector<char*> chain;

char* prepare_block() {
	char *ret = new char [block_size];
	//nonce
	memset(ret + 32, '0', sizeof(char) * 32);
	//pre-hash
	if (chain.size()) {
		memcpy(ret + 64, chain.back() + 64, sizeof(char) * 32);
	} else {
		memcpy(ret + 64, HASH0, sizeof(char) * 32);
	}
	//height
	int_to_byte_array32(chain.size(), ret + 96);
	//miner_add
	memcpy(ret + 128, MINER_ADD, sizeof(char) * 32);
	return ret;
}

void message_loop() {
	bool closed = false;
	while (!closed) {
		current_block = prepare_block();
		num_tx = 0;
		while (!closed && num_tx < numtxinblock) {
			int m_size;
			const char *m = get_message(m_size);
			switch (get_type(m[0])) {
				case TRANSACTION: {
					const char *tx = m + 1;
					int sender = get_sender_id(tx);
					if (sender != -1) {
						int receiver = get_receiver_id(tx);
						if (receiver != -1) {
							int amount = get_amount(tx);
							if (amount != -1) {
								if (UTXO[sender] >= amount) {
									long long timestamp = get_timestamp(tx);
									if (!is_duplicate_tx(sender, receiver, amount, timestamp)) {
										insert_tx(sender, receiver, amount, timestamp);
										UTXO[sender] -= amount;
										UTXO[receiver] += amount;
										memcpy(current_block + 160 + num_tx * tx_size, tx, tx_size);
										++num_tx;
										broadcast(m, m_size);
									}
								}
							}
						}
					}
					break;
				}
				case CLOSE: {
					closed = true;
					broadcast(m, m_size);
					break;
				}
				case BLOCK: {
					broadcast(m, m_size);
					break;
				}
				case GET_BLOCK: {
					int h = get_block_height(m);
					if (h != -1 && h < chain.size()) {
						reply("2", 1);
						reply(chain[h], block_size);
					}
					break;
				}
			}
		}
		if (num_tx == numtxinblock) {
			if (use_rewind) {
				rewind_mine(current_block);
			} else {
				mine(current_block);
			}
			rotate(current_block, current_block + 32, current_block + 96);
			chain.push_back(current_block);
			broadcast("2", 1);
			broadcast(current_block, block_size);
			current_block = NULL;
			num_tx = 0;
		}
		if (closed && current_block) {
			delete current_block;
		}
	}
}

void close_core() {
	while (chain.size()) {
		delete chain.back();
		chain.pop_back();
	}
}
