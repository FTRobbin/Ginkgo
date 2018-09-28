#ifndef CORE_H
#define CORE_H

extern int numtxinblock, numcores;
 
extern bool use_rewind;

void init_UTXO();

void message_loop();

void close_core();

#endif
