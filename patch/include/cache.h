#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

void inval_D_cache(uint32_t addr, uint32_t len_in_blocks);
void inval_I_cache(uint32_t addr, uint32_t len_in_blocks);
void inval_DI_cache(uint32_t addr, uint32_t len_in_blocks);

#endif