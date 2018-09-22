#include <ctime>
#include <string>
#include <queue>
#include <set>
#include <map>

#include <algorithm>

#include "core.h"
#include "addr.h"
#include "util.h"
#include "miner.h"
#include "network.h"

#include "test.h"

using namespace std;

const int INIT_AMOUNT = 100000;

map<SHA256H, int> member;

int UTXO[100];

void init_UTXO() {
	for (int i = 0; i < 100; ++i) {
		member[string(ACCOUNT_ADDS[i], 32)] = i;
		UTXO[i] = INIT_AMOUNT;
	}
}

typedef byte_array TX;

int get_id(const byte_array &hash) {
	if (member.count(hash)) {
		return member[hash];
	} else {
		return -1;
	}
}

int get_sender_id(const TX &tx) {
	return get_id(tx.substr(0, 32));
}

int get_receiver_id(const TX &tx) {
	return get_id(tx.substr(32, 32));
}

int get_amount(const TX &tx) {
	byte_array amt = tx.substr(64, 32);
	if (overflow_10_9(amt)) {
		return -1;
	} else {
		return byte_array_to_int(amt);
	}
}

int get_block_height(const message &m) {
	byte_array he = m.substr(1);
	if (overflow_10_9(he)) {
		return -1;
	} else {
		return byte_array_to_int(he);
	}
}

queue<TX> q;

set<TX> history;

typedef byte_array block;

vector<block> chain;

void message_loop() {
	bool closed = false;
	while (!closed) {
		while (!closed && q.size() < numtxinblock) {
			message m = get_message();
			switch (get_type(m[0])) {
				case TRANSACTION: {
					TX tx = m.substr(1);
					int sender = get_sender_id(tx);
					if (sender != -1) {
						int receiver = get_receiver_id(tx);
						if (receiver != -1) {
							int amount = get_amount(tx);
							if (amount != -1) {
								if (UTXO[sender] >= amount) {
									if (!history.count(tx)) {
										history.insert(tx);
										UTXO[sender] -= amount;
										UTXO[receiver] += amount;
										q.push(tx);
										broadcast(m);
									}
								}
							}
						}
					}
					break;
				}
				case CLOSE: {
					closed = true;
					broadcast(m);
					break;
				}
				case BLOCK: {
					broadcast(m);
					break;
				}
				case GET_BLOCK: {
					int h = get_block_height(m);
					if (h != -1 && h < chain.size()) {
						reply("2");
						reply(chain[h]);
					}
					break;
				}
			}
		}
		if (q.size() == numtxinblock) {
			clock_t start = clock();
			block nb = string(32 * 5 + 128 * (numtxinblock), '0');
			byte_array prior_hash = chain.size() ? chain.back().substr(64, 32) : string(HASH0, 32);
			inplace_replace(nb, 64, 32, prior_hash);
			inplace_replace(nb, 96, 32, int_to_byte_array32(chain.size()));
			inplace_replace(nb, 128, 32, string(MINER_ADD, 32));
			for (int i = 0; q.size(); q.pop(), ++i) {
				inplace_replace(nb, 160 + 128 * i, 128, q.front());
			}
			mine(nb);
			rotate(nb.begin(), nb.begin() + 32, nb.begin() + 96);
			chain.push_back(nb);
			if (verbose) {
				printf("Mine block %d: %.6f seconds\n", (int)chain.size() - 1, (double)(clock() - start) / CLOCKS_PER_SEC);
			}
			broadcast("2");
			broadcast(nb);
		}
	}
}
