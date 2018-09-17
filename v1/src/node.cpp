#include<cassert>
#include<set>
#include<map>
#include<queue>
#include<string>
#include<vector>
#include<cstdio>
#include<cstring>
#include<iostream>
#include<algorithm>

#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#include"sha256.h"

using namespace std;

const char* MY_NETID = "hn332";

typedef string byte_array;

typedef byte_array SHA256b;

bool verbose = false;

void debug_bytes(byte_array ba) {
	printf("[");
	for (int i = 0; i < (int)ba.size(); ++i) {
		printf("%u%c", (unsigned)((unsigned char)ba[i]), i == (int)ba.size() - 1 ? ']' : ' ');
	}
	printf("\n");
}

void debug_bytes_hex(byte_array ba) {
	printf("[");
	for (int i = 0; i < (int)ba.size(); ++i) {
		for (int j = 0; j < 2; ++j) {
			int d = (unsigned char)ba[i] >> (4 * (1 - j)) & 15;
			printf("%c", d < 10 ? '0' + d : 'a' + (d - 10));
		}
	}
	printf("]\n");
}

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

byte_array int_to_byte_array32(unsigned i) {
	byte_array ret32 = int_to_byte_array(i);
	while (ret32.size() < 32) {
		ret32 = string("0") + ret32;
	}
	return ret32;
}

byte_array string_to_byte_array(string s) {
	return s;
}

unsigned byte_array_to_int(byte_array ba) {
	unsigned ret = 0;
	for (int i = 0; i < ba.length(); ++i) {
		ret = ret * 10 + (ba[i] - '0');
	}
	return ret;
}

int port = 9000, numtxinblock = 50000, difficulty = 1, numcores = 1;

vector<int> peers;

void process_arguments(int argn, char *args[]) {
	for (int i = 1; i < argn; ++i) {
		char *s = args[i];
		if (strcmp(s, "--port") == 0) {
			++i;
			sscanf(args[i], "%d", &port);
		} else if (strcmp(s, "--peers") == 0) {
			++i;	
			int l = strlen(args[i]);
			for (int j = 0, tmp = 0; j <= l; ++j) {
				if ('0' <= args[i][j] && args[i][j] <= '9') {
					tmp = tmp * 10 + (args[i][j] - '0');
				} else {
					peers.push_back(tmp);
					tmp = 0;
				}
			}
		} else if (strcmp(s, "--numtxinblock") == 0) {
			++i;
			sscanf(args[i], "%d", &numtxinblock);
		} else if (strcmp(s, "--difficulty") == 0) {
			++i;
			sscanf(args[i], "%d", &difficulty);
		} else if (strcmp(s, "--numcores") == 0) {
			++i;
			sscanf(args[i], "%d", &numcores);
		} else if (strcmp(s, "--verbose") == 0) {
			verbose = true;
		} else {
			assert(false);
		}
	}
}

map<SHA256b, int> UTXO; //does int suffice

void init_UTXO() {
	for (int i = 0; i < 100; ++i) {
		SHA256b account = sha256(int_to_byte_array(i));
		//debug_bytes_hex(account);
		UTXO[account] = 100000;	
	}
}

enum message_type {TRANSACTION, CLOSE, BLOCK, GET_BLOCK};

struct message {
	byte_array data;

	message(byte_array data) : data(data) {}

	unsigned get_type() {
		return data[0] - '0';
	}

	byte_array get_tx() {
		return data.substr(1);
	}

	byte_array get_block_height() {
		return data.substr(1);
	}
};

struct TX {
	byte_array sender, receiver, amount, timestamp;	

	TX (byte_array data) {
		sender = data.substr(0, 32);
		receiver = data.substr(32, 32);
		amount = data.substr(64, 32);
		timestamp = data.substr(96, 32);	
	}

	unsigned get_amount() {
		return byte_array_to_int(amount);
	}

	byte_array get_raw() const {
		return sender + receiver + amount + timestamp;
	}

	bool amount_overflow() const {
		for (int i = 0; i < 32 - 9; ++i) {
			if (amount[i] != '0') {
				return true;
			}
		}
		for (int i = 32 - 9; i < 32; ++i) {
			if (amount[i] < '0' || amount[i] > '9') {
				return true;
			}
		}
		return false;
	}
};

queue<TX> q;

set<TX> tx_history;

bool operator < (const TX &a, const TX &b) {
	return a.get_raw() < b.get_raw();
}

int get_size_type(int t) {
	switch (t - '0') {
		case TRANSACTION: {
							  return 1 + 32 * 4;
						  }
		case CLOSE: {
						return 1;
					}
		case BLOCK: {
						return 1 + 32 * 5 + 128 * numtxinblock;
					}
		case GET_BLOCK: {
							return 1 + 32;
						}
		default : {
					  exit(-1);
				  }
	}
}

struct block {
	byte_array nonce, prior_hash, hash, block_height, miner_address;
	vector<TX> block_data;

	message get_message() {
		byte_array data = "2";
		data += nonce + prior_hash + hash + block_height + miner_address;
		for (int i = 0; i < (int)block_data.size(); ++i) {
			data += block_data[i].get_raw();
		}
		return message(data);
	}

	byte_array get_hash() {
		byte_array raw = nonce + prior_hash + block_height + miner_address;
		for (int i = 0; i < (int)block_data.size(); ++i) {
			raw += block_data[i].get_raw();
		}
		return sha256(raw);
	}

	int get_difficulty() {
		int ret = 0;
		for (int i = 0, flag = false; i < (int)hash.size() && !flag; ++i) {
			for (int j = 7; j >= 0 && !flag; --j, ++ret) {
				if (hash[i] & (1 << j)) {
					flag = true;
				}
			}
		}	
		return ret;
	}
};

vector<block> chain;

vector<int> peer_conns;

int last_connection = -1;

void init_network() {
	//connect to all peers
	for (int i = 0; i < (int)peers.size(); ++i) {	
		int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		//printf("sockfd = %d\n", sockfd);
		if (sockfd == -1) {
			perror("socket failed");
			exit(-1);
		}
		struct sockaddr_in addr;
		int addr_size = sizeof(sockaddr_in);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(peers[i]);
		if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
			perror("inet_pton failed");
			exit(-1);
		}
		if (connect(sockfd, (sockaddr *) &addr, addr_size) == -1) {
			perror("connect failed");
			exit(-1);
		}
		peer_conns.push_back(sockfd);
	}
}

message get_message() {
	static int sockfd;
	static sockaddr_in addr;
	static int addr_size;
	if (last_connection == -1) {
		//listen to server		
		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		//printf("sockfd = %d\n", sockfd);
		if (sockfd == -1) {
			perror("socket failed");
			exit(-1);
		}
		sockaddr_in addr;
		addr_size = sizeof(sockaddr_in);
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);
		bind(sockfd, (sockaddr *) &addr, addr_size);
		{
			//perror("bind failed");
			//exit(-1);
		}
		if (listen(sockfd, 100) == -1) {
			perror("listen failed");
			exit(-1);
		}
	}
	char type[2], *buf;
	while (last_connection == -1 || read(last_connection, type, 1) == 0) {
		last_connection = accept(sockfd, (sockaddr *) &addr, (socklen_t *) &addr_size);
		if (last_connection < 0) {
			perror("accpet failed");
			exit(-1);
		}
	}
	int bsize = get_size_type(type[0]);
	byte_array data = "";
	if (bsize > 1) {
		buf = new char[bsize];
		int pos = 0;
		while (pos < bsize - 1) {
			int delta = read(last_connection, buf + pos, bsize - 1 - pos);
			pos += delta;
		}
		data = string(type) + string(buf, bsize - 1);
	} else {
		data = string(type);
	}
	return message(data);
}

void broadcast(message m) {
	for (int i = 0; i < (int)peer_conns.size(); ++i) {
		//debug_bytes(m.data);
		send(peer_conns[i], m.data.c_str(), m.data.size(), 0);
	}
}

void reply(message m) {
	send(last_connection, m.data.c_str(), m.data.size(), 0);
}

int main(int argn, char *args[]) {
	process_arguments(argn, args);
	init_UTXO();
	init_network();
	//main loop
	bool closed = false;
	while (!closed) {
		while (!closed && q.size() < numtxinblock) {
			message m = get_message();
			if (verbose) {
				debug_bytes(m.data);
			}
			switch (m.get_type()) {
				case TRANSACTION: {
									  TX tx = TX(m.get_tx());
									  if (UTXO.count(tx.sender) && UTXO.count(tx.receiver)
										  && !tx.amount_overflow() && UTXO[tx.sender] >= tx.get_amount() 
										  && !tx_history.count(tx)) {
										  UTXO[tx.sender] -= tx.get_amount();
										  UTXO[tx.receiver] += tx.get_amount();
										  q.push(tx);
										  tx_history.insert(tx);
										  broadcast(m);
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
									unsigned h = byte_array_to_int(m.get_block_height());
									if (h < chain.size()) {
										message mb = chain[h].get_message();
										reply(mb);
									}
									break;
								}
				default: {
					exit(-1);
				}
			}
		}
		if (q.size() == numtxinblock) {
			block b;
			b.prior_hash = chain.size() ? chain.back().hash : sha256(int_to_byte_array(0));
			b.block_height = int_to_byte_array32(chain.size());
			b.miner_address = sha256(string_to_byte_array(MY_NETID));
			while (q.size()) {
				b.block_data.push_back(q.front());
				q.pop();
			}
			unsigned it = 0;
			while (true) {
				b.nonce = int_to_byte_array32(it++);
				b.hash = b.get_hash();
				if (b.get_difficulty() >= difficulty) {
					break;
				}
			}
			chain.push_back(b);
			message mb = b.get_message();
			broadcast(mb);
		}
	}
	return 0;
}
