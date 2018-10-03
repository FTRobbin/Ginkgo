#ifndef CORE_H
#define CORE_H

#include <vector>

extern int numtxinblock, numcores;

extern std::vector<char*> chain;
 
void *core_main(void *);

#endif
