#include "common.h"

/* Functions from binary */

int (*printf)(const char *fmt, ...) = (void*)0xa04a18;
event_t* (*add_event)(int trigger_time, void *callback, int param) = (void*)0xa04ff4;
void (*lut_add_entry)(uint32_t addr, uint32_t size, void *ptr_load_handler, void *ptr_store_handler) = (void*)0xa069a4;

event_t event_buffer[0x14];

int valid_addr(uint32_t addr)
{
	if ((addr - 0x00000000) < 0x00700000) return 1; //IOP RAM and mirrors
	if ((addr - 0x00A00000) < 0x00100000) return 1; //DECKARD binary
	if ((addr - 0x00B40000) < 0x00000800) return 1;	//PS1DRV
	if ((addr - 0x00BE0000) < 0x00001000) return 1; //DECKARD variables
	if ((addr - 0x00BFF000) < 0x00001000) return 1; //PPC stack
	if ((addr - 0x12000000) < 0x01000000) return 1; //Dev10 & more
	if ((addr - 0x13000000) < 0x01000000) return 1; //Dev9
	if ((addr - 0x1D000000) < 0x00000400) return 1; //SIF SBUS
	if ((addr - 0x1E000000) < 0x01000000) return 1; //DVD ROM
	if ((addr - 0x1F402000) < 0x00000400) return 1; //1kb cache
	if ((addr - 0x1F7FFC00) < 0x00000400) return 1; //1kb cache
	if ((addr - 0x1F808000) < 0x00000400) return 1; //SIO2 (iLink, after it unmapped)
	if ((addr - 0x1F900000) < 0x00001000) return 1; //SPU2
	if ((addr - 0x1FC00000) < 0x00400000) return 1; //BOOT ROM
	if ((addr - 0xFFE00000) < 0x00200000) return 1;	//DECKARD bootrom

	return 0;
}