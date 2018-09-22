#ifndef NETWORK_H
#define NETWORK_H

#include <string>

const int MAX_BACKLOG = 100;

void init_network();

typedef std::string message;

enum message_type {INVALID, TRANSACTION = '0', CLOSE = '1', BLOCK = '2', GET_BLOCK = '3'};

message_type get_type(const char &);

message get_message();

void broadcast(const message &);

void reply(const message &);

void close_network();

#endif
