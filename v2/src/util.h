#ifndef UTIL_H
#define UTIL_H

#include<string>

bool overflow_10_9 (const byte_array &);

int byte_array_to_int(const byte_array &);

byte_array int_to_byte_array32(int);

void inplace_replace(std::string &, int, int, const std::string &);

#endif
