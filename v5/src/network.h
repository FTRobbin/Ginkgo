#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include <netinet/ip.h>

#include "cqueue.h"

extern cqueue msg_queue, broadcast_queue;

const int MAX_BACKLOG = 1024 - 2;

extern int self_port;

extern std::vector<int> peer_ports;

void *network_in_main(void *);

void *network_out_main(void *);

enum message_type {INVALID, TRANSACTION = '0', CLOSE = '1', BLOCK = '2', GET_BLOCK = '3'};

message_type get_type(const char &);

#endif
