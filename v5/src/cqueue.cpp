#include <cstdio>
#include <pthread.h>

#include "cqueue.h"

cqueue::cqueue() {
	head = tail = 0;
	q_lock = PTHREAD_MUTEX_INITIALIZER;
	queue_full = PTHREAD_COND_INITIALIZER;
	queue_empty = PTHREAD_COND_INITIALIZER;
}

bool cqueue::isfull() {
	return head == tail + 1 || (head == 0 && tail == MAX_CQUEUE_SIZE - 1);
}

bool cqueue::isempty() {
	return head == tail;
}

void cqueue::push(char *e) {
	pthread_mutex_lock(&q_lock);
	while (isfull()) {
		pthread_cond_wait(&queue_full, &q_lock);
	}
	storage[tail] = e;
	if ((++tail) == MAX_CQUEUE_SIZE) {
		tail = 0;
	}
	pthread_cond_signal(&queue_empty);
	pthread_mutex_unlock(&q_lock);
}

char* cqueue::pop() {
	pthread_mutex_lock(&q_lock);
	while (isempty()) {
		pthread_cond_wait(&queue_empty, &q_lock);
	}
	++head;
	char *ret = storage[head - 1];
	if (head == MAX_CQUEUE_SIZE) {
		head = 0;
	}
	pthread_cond_signal(&queue_full);
	pthread_mutex_unlock(&q_lock);
	return ret;
}

