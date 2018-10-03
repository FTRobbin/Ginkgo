#ifndef MINER_H
#define MINER_H

#include <pthread.h>

extern bool use_rewind;

extern int difficulty;

extern int miner_ptr;

extern pthread_mutex_t miner_lock;
extern pthread_cond_t has_unmined;

void *miner_main(void *);

#endif
