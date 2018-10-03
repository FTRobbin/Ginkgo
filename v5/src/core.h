#ifndef CORE_H
#define CORE_H

#include <vector>

extern int numtxinblock;

extern std::vector<char*> chain;
 
void *core_main(void *);

#endif
