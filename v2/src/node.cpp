#include <cstdio>
#include <cstring>

#include <vector>

#include <stdexcept>

#include "core.h"
#include "network.h"

using namespace std;

bool verbose = false;

int self_port = -1;
int numtxinblock = 50000;
int difficulty = 0;
int numcores = 1;

bool use_rewind = true;

std::vector<int> peer_ports;

void process_args(int argn, char *args[]) {
	for (int i = 1; i < argn; ++i) {
		char *s = args[i];
		if (strcmp(s, "--port") == 0) {
			++i;
			sscanf(args[i], "%d", &self_port);
		} else if (strcmp(s, "--peers") == 0) {
			++i;	
			int l = strlen(args[i]);
			for (int j = 0, tmp = 0; j <= l; ++j) {
				if ('0' <= args[i][j] && args[i][j] <= '9') {
					tmp = tmp * 10 + (args[i][j] - '0');
				} else if (j == l || args[i][j] == ',') {
					peer_ports.push_back(tmp);
					tmp = 0;
				} else {
					fprintf(stderr, "Unexpected peer format: %s\n", args[i]);
					throw runtime_error("");
				}
			}
		} else if (strcmp(s, "--numtxinblock") == 0) {
			++i;
			sscanf(args[i], "%d", &numtxinblock);
			if (numtxinblock <= 10) {
				use_rewind = false;
			}
		} else if (strcmp(s, "--difficulty") == 0) {
			++i;
			sscanf(args[i], "%d", &difficulty);
		} else if (strcmp(s, "--numcores") == 0) {
			++i;
			sscanf(args[i], "%d", &numcores);
		} else if (strcmp(s, "--verbose") == 0) {
			verbose = true;
		} else if (strcmp(s, "--no-rewind") == 0) {
			use_rewind = false;
		} else {
			fprintf(stderr, "Unexpected argument: %s\n", args[i]);
			throw runtime_error("");
		}
	}
	if (self_port == -1) {
		fprintf(stderr, "Port number not given");
		throw runtime_error("");
	}
}

void init_node() {
	init_UTXO();
	init_network();
}

void close_all() {
	close_network();
}

int main(int argn, char *args[]) {
	process_args(argn, args);
	init_node();
	message_loop();
	close_all();
	return 0;
}
