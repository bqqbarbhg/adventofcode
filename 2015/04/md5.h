#ifndef INCLUDED_MD5_H
#define INCLUDED_MD5_H

#define MD5_HASH_BYTES 16
#define MD5_DUMP_LENGTH 36

#include <stdint.h>


void md5_hash(void *result, const void *data, size_t length);
void md5_dump(char *buffer, const void *hash);

#endif
