#ifndef CQUEUE_H
#define CQUEUE_H

#include <pthread.h>

//Blocking concurrent fixed-size queue

const int MAX_CQUEUE_SIZE = 1048576 * 2 - 2;

struct cqueue {
	int head, tail;
	char * storage[MAX_CQUEUE_SIZE];

	pthread_mutex_t q_lock;

	pthread_cond_t queue_full;
	pthread_cond_t queue_empty;

	cqueue();

	bool isfull();

	bool isempty();

	void push(char *);

	char* pop();
};

#endif
