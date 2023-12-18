#include "cache.h"

//each block is 32 bytes
void inval_D_cache(uint32_t addr, uint32_t len_in_blocks)
{
	for (int i = 0; i < len_in_blocks; i++) {
		asm volatile(
			"dcbi 0, %[addr];"
			"sync;"
			::[addr] "r" (addr));
		addr += 0x20;
	}
}

void inval_I_cache(uint32_t addr, uint32_t len_in_blocks)
{
	for (int i = 0; i < len_in_blocks; i++) {
		asm volatile(
			"icbi 0, %[addr];"
			"isync;"
			::[addr] "r" (addr));
		addr += 0x20;
	}
}

void inval_DI_cache(uint32_t addr, uint32_t len_in_blocks)
{
	for (int i = 0; i < len_in_blocks; i++) {
		asm volatile(
			"li 0, 0x0;"
			"dcbi 0, %[addr];"
			"sync;"
			"icbi 0, %[addr];"
			"isync;"
			::[addr] "r" (addr):"0");
		addr += 0x20;
	}
}