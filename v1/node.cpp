#include<map>
#include<queue>
#include<string>
#include<vector>
#include<cstdio>
#include<cstring>
#include<iostream>
#include<algorithm>

using namespace std;

const char* MY_NETID = "hn332";

typedef string byte_array;

typedef byte_array SHA256;

byte_array int_to_byte_array(unsigned i) {
	// 0 --> "0"
	byte_array ret = "";
	//TODO : what's the specification
	return ret;
}

byte_array int_to_byte_array32(unsigned i) {
	byte_array ret32 = "00000000000000000000000000000000";
	//TODO : what's the specification
	return ret32;
}

byte_array string_to_byte_array(string s) {
	//TODO
	return s;
}

unsigned byte_array_to_int(byte_array ba) {
	//TODO : oh
	return 0;
}

SHA256 sha256(byte_array x) {
	// sha256 ("0") = 5feceb66ffc86f38d952786c6d696c79c2dbc239dd4e91b46729d73a27fb57e9
	//TODO : real sha256
	return x;
}

int port = 9000, numtxinblock = 2, difficulty = 1;

vector<int> peers;

void process_arguments(int argn, char *args[]) {
	//TODO : parse the arguments
}

map<SHA256, int> UTXO; //int suffices

void init_UTXO() {
	for (int i = 0; i < 100; ++i) {
		SHA256 account = sha256(int_to_byte_array(i));
		UTXO[account] = 100000;	
	}
}

enum message_type {TRANSACTION, CLOSE, BLOCK, GET_BLOCK};

struct message {
	byte_array data;

	message(byte_array data) : data(data) {}

	unsigned get_type() {
		return data[0];
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

	byte_array get_raw() {
		return sender + receiver + amount + timestamp;
	}
};

queue<TX> q;

struct block {
	byte_array nonce, prior_hash, hash, block_height, miner_address;
	vector<TX> block_data;

	message get_message() {
		byte_array data = "\0";
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

message get_message() {
	//TODO: real input from network
	byte_array data;
	cin >> data;
	return message(data);
}

void forward(message m) {
	//TODO: real output to network
	cout << m.data << endl;
}

void broadcast(message m) {
	//TODO: real output to network
	cout << m.data << endl;
}

int main(int argn, char *args[]) {
	process_arguments(argn, args);
	init_UTXO();
	//main loop
	bool closed = false;
	while (!closed) {
		while (!closed && q.size() < numtxinblock) {
			message m = get_message();
			switch (m.get_type()) {
				case TRANSACTION: {
					TX tx = TX(m.get_tx());
					if (UTXO[tx.sender] >= tx.get_amount()) {
						UTXO[tx.sender] -= tx.get_amount();
						UTXO[tx.receiver] += tx.get_amount();
						q.push(tx);
						forward(m);
					}
					break;
				}
				case CLOSE: {
					closed = true;
					forward(m);
					break;
				}
				case BLOCK: {
					//Do nothing
					break;
				}
				case GET_BLOCK: {
					unsigned h = byte_array_to_int(m.get_block_height());
					if (h < chain.size()) {
						message mb = chain[h].get_message();
						//TODO: return to the sender
						broadcast(mb);
					}
					break;
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
