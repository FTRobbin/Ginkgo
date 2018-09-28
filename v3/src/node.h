#ifndef NODE_H
#define NODE_H

#include <vector>

extern bool verbose; 

extern int self_port, numtxinblock, difficulty, numcores;

extern size_t tx_size, block_size;
 
extern std::vector<int> peer_ports;

extern bool use_rewind;

#endif
