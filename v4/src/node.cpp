#include <pthread.h>
#include <cstdio>
#include <cstring>

#include <vector>

#include <stdexcept>

#include "core.h"
#include "network.h"
#include "miner.h"
#include "util.h"

using namespace std;

bool verbose = false;
bool isclosed = false;

int self_port = -1;
int numtxinblock = 50000;
int difficulty = 0;
int numcores = 1;

size_t tx_size = 128, block_size;

bool use_rewind = true;

vector<int> peer_ports;

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
		fprintf(stderr, "Port number not given\n");
		throw runtime_error("");
	}
	block_size = 5 * 32 + tx_size * numtxinblock;
}

int main(int argn, char *args[]) {
	process_args(argn, args);
	pthread_t network_in, network_out, core, miner;
	pthread_create(&network_out, NULL, &network_out_main, NULL);
	pthread_create(&miner, NULL, &miner_main, NULL);
	pthread_create(&core, NULL, &core_main, NULL);
	pthread_create(&network_in, NULL, &network_in_main, NULL);
	pthread_join(network_in, NULL);
	pthread_join(core, NULL);
	pthread_join(miner, NULL);
	pthread_join(network_out, NULL);
	return 0;
}
