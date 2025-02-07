#pragma once

#include <cryptonote_basic/cryptonote_basic.h>
#include <cryptonote_basic/cryptonote_format_utils.h>

uint32_t convert_blob(const char *blob, size_t len, char *out);
bool validate_address(const char *addr, size_t len);
uint64_t slow_memmem(const void* start_buff, size_t buflen,const void* pat,size_t patlen);
uint64_t rx_seedheight(uint64_t height);
void rx_slow_hash(uint64_t height, uint64_t seedHeight, const char* seedHash, const char* blob, uint64_t blobSize, char* output, int param1, int param2);