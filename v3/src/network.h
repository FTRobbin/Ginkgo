#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

const int MAX_BACKLOG = 100;

extern int self_port;

extern std::vector<int> peer_ports;

void init_network();

enum message_type {INVALID, TRANSACTION = '0', CLOSE = '1', BLOCK = '2', GET_BLOCK = '3'};

message_type get_type(const char &);

const char* get_message(int &);

void broadcast(const char* , const int);

void reply(const char* , const int);

void close_network();

#endif
