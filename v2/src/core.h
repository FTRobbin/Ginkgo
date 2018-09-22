#ifndef CORE_H
#define CORE_H

#include <vector>
#include <string>

extern bool verbose; 

extern int self_port, numtxinblock, difficulty, numcores;
 
extern std::vector<int> peer_ports;

typedef std::string byte_array;

typedef byte_array SHA256H;

void init_UTXO();

void message_loop();

#endif
