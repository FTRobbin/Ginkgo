#ifndef UTIL_H
#define UTIL_H

extern bool verbose;

extern size_t tx_size, block_size;

bool overflow_10_9 (const char *);

int byte_array32_to_int(const char *);

long long byte_array_to_long_long(const char *, const int);

void int_to_byte_array32(const int, char *);

#endif
