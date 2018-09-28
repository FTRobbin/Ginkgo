#include <cstdio>

#include <stdexcept>
#include <vector>

#include <unistd.h>
#include <netinet/ip.h>

#include "network.h"
#include "util.h"

using namespace std;

message_type get_type(const char &ch) {
	if ('0' <= ch && ch <= '3') {
		return message_type(ch);
	} else {
		return INVALID;
	}
}

int get_type_size(const message_type &mt) {
	switch (mt) {
		case INVALID: return 0;
		case TRANSACTION: return 1 + tx_size;
		case CLOSE: return 1;
		case BLOCK: return 1 + block_size;
		case GET_BLOCK: return 1 + 32;
	}
}

vector<int> peer_socks;

int get_sockfd() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		perror("Failed to create new socket descriptor");
		throw runtime_error("");
	}
	return sockfd;
}

sockaddr_in get_sockaddr(int port) {
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	return addr;
}

int self_sock;

void init_self() {
	self_sock = get_sockfd();
	/*
	int enable = 1;
	if (setsockopt(self_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("Failed to setsockopt(SO_REUSEADDR)");
		throw runtime_error("");
	}
	*/
	sockaddr_in addr = get_sockaddr(self_port);
	if (bind(self_sock, (sockaddr *) &addr, sizeof(sockaddr_in)) == -1) {
		perror("Failed to bind on self port");
		throw runtime_error("");
	}
	if (listen(self_sock, MAX_BACKLOG) == -1) {
		perror("Failed to listen on self port");
		throw runtime_error("");
	}
}

void init_peers() {
	for (int i = 0; i < (int)peer_ports.size(); ++i) {
		int sockfd = get_sockfd();	
		sockaddr_in addr = get_sockaddr(peer_ports[i]);
		if (connect(sockfd, (sockaddr *) &addr, sizeof(sockaddr_in)) == -1) {
			perror("Failed to connect to peer");
			continue;
		}
		peer_socks.push_back(sockfd);
	}
}

int buf_size = 0;
char *buf;

void init_buffers() {
	for (char ch = TRANSACTION; ch <= GET_BLOCK; ++ch) {
		buf_size = max(buf_size, get_type_size(get_type(ch)));
	}
	buf = new char[buf_size + 1];
}

void init_network() {
	init_self();
	init_peers();
	init_buffers();
}

int curr_sock = -1;

int get_new_curr_sock() {
	int sockfd = accept(self_sock, NULL, NULL);
	if (sockfd == -1) {
		perror("Failed to accept incoming connection");
		throw runtime_error("");
	}
	return sockfd;
}

void close_curr_sock(int &curr_sock) {
	if (close(curr_sock) == -1) {
		perror("Failed to close current socket");
		throw runtime_error("");
	}
	curr_sock = -1;
}

const char* get_message(int &msg_size) {
	while (true) {
		if (curr_sock == -1) {
			curr_sock = get_new_curr_sock();
		}
		int ret = recv(curr_sock, buf, 1, MSG_PEEK | MSG_WAITALL);
		if (ret == -1) {
			perror("Failed to recv from listened port");
			close_curr_sock(curr_sock);
			continue;
		}
		if (ret == 0) {
			close_curr_sock(curr_sock);
			continue;
		}
		message_type mt = get_type(buf[0]);
		if (mt == INVALID) {
			fprintf(stderr, "Received invalid message type %u", (unsigned)(unsigned char)buf[0]);
			close_curr_sock(curr_sock);
			continue;
		}
		msg_size = get_type_size(mt);
		ret = recv(curr_sock, buf, msg_size, MSG_WAITALL);
		if (ret == -1) {
			perror("Failed to recv from listened port");
			close_curr_sock(curr_sock);
			continue;
		}
		if (ret != msg_size) {
			fprintf(stderr, "Received incomplete message");
			close_curr_sock(curr_sock);
			continue;
		}
		return buf;
	}
}

void broadcast(const char *m, const int size) {
	for (int i = 0; i < (int)peer_socks.size(); ++i) {
		if (peer_socks[i] != -1) {
			if (send(peer_socks[i], m, size, 0) == -1) {
				perror("Failed to send to peer");
				peer_socks[i] = -1;
			}
		}
	}
}

void reply(const char *m, const int size) {
	if (send(curr_sock, m, size, 0) == -1) {
		perror("Failed to reply");
		close_curr_sock(curr_sock);
	}
}

void close_network() {
	delete buf;
	for (int i = 0; i < (int)peer_socks.size(); ++i) {
		if (peer_socks[i] != -1) {
			if (close(peer_socks[i]) == -1) {
				perror("Failed to close the peer socket");
			}
		}
	}
	while (curr_sock != -1 && recv(curr_sock, buf, buf_size, 0) > 0);
	if (curr_sock != -1 && close(curr_sock) == -1) {
		perror("Failed to close the curret socket");
		throw runtime_error("");
	}
	if (close(self_sock) == -1) {
		perror("Failed to close the listening socket");
		throw runtime_error("");
	}
}
