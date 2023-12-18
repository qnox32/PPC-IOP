#include <stdint.h>

#include "debug.h"
#include "common.h"
#include "cache.h"

#define LOAD_LUT_ADDR 0xA10000
#define STORE_LUT_ADDR 0xA12000

uint32_t (*debug_apu_reg_get)(uint32_t reg) = (void*)0xa031b0;
uint32_t (*debug_gte_reg_get)(uint32_t reg) = (void*)0xa031a0;

uint32_t (*debug_apu_reg_set)(uint32_t reg, uint32_t value) = (void*)0xa031a8;
uint32_t (*debug_gte_reg_set)(uint32_t reg, uint32_t value) = (void*)0xa03198;

static uint8_t reset_func_idx;
static void *reset_func[0x10];

//TODO: include and clean up code from old debug_ppc project

/*
* This core does not support mfdcrx and mtdcrx instructions so self modifying
* code is used instead
*/
uint32_t debug_dcr_get(uint16_t dcr) 
{
	uint32_t value;
	static uint32_t instr[3];

	if (dcr > 0x3FF) {
		printf("DCR range 0x0-0x3FF");
	}

	//DCRN ← DCRF5:9 || DCRF0:4
	uint32_t dcrn = (dcr & 0x1F) << 5 | (dcr & 0x3E0) >> 5;
	
	instr[0] = 0x7c600286 | (dcrn << 11); 	//mfdcr r3, dcrn;
	instr[1] = 0x4e800020; 					//blr

	uint32_t (*func)() = (void*)(&instr);

	inval_DI_cache((uint32_t)&instr, 1);

	value = func();

	return value;
}

void debug_dcr_set(uint16_t dcr, uint32_t value)
{
	static uint32_t instr[3];

	if (dcr > 0x3FF) {
		printf("DCR range 0x0-0x3FF");
	}

	//DCRN ← DCRF5:9 || DCRF0:4
	uint32_t dcrn = (dcr & 0x1F) << 5 | (dcr & 0x3E0) >> 5;
	
	instr[0] = 0x7c600386 | (dcrn << 11); 	//mfdcr r3, dcrn;
	instr[1] = 0x4e800020; 					//blr

	//r3 is always first param
	void (*func)(uint32_t value) = (void*)(&instr);

	inval_DI_cache((uint32_t)&instr, 1);

	func(value);
}

//uint32_t debug_ppc_reg_get(uint8_t reg) {}
//uint32_t debug_ppc_sreg_get(uint8_t reg) {}

void debug_ppc_reg_set(uint8_t reg, uint32_t value) {}
void debug_ppc_sreg_set(uint8_t sreg, uint32_t value) {}


//each entry in the table spans 0x10 (16 bytes.)
//ex. an entry for 0x1F808300 would cover 0x1F808300 to 0x1F808310.
void debug_lut_add_entry(uint32_t lut_addr, uint32_t addr, uint32_t len, void *func_ptr)
{
    uint32_t size = len >> 4;
    
    if (size == 0) {
        printf("len too short for entry, minimum size is 0x10\n");
    }

    for (int i = 0; i < len; i+=4) {
        *(uint32_t*)(lut_addr+i) = (uint32_t)func_ptr;
    }
}

//TODO:
void debug_prefix_dispatch(uint32_t addr, uint32_t arg2, uint32_t arg3){}

void debug_patch_dev(uint8_t patch_type, uint32_t addr, uint32_t len, void *load_func, void *store_func)
{
    if (patch_type == PATCH_PREFIX) {

    }

    if (patch_type == PATCH_POSTFIX) {

    }

    if (patch_type == PATCH_REPLACE) {
        if (load_func != 0x0)
            debug_lut_add_entry(LOAD_LUT_ADDR, addr, len, load_func);

        if (store_func != 0x0)
            debug_lut_add_entry(STORE_LUT_ADDR, addr, len, store_func);
    }
}

void debug_hook_reset()
{
	uint32_t ba = 0x48000002 | (uint32_t)&debug_reset_handler;
	*(uint32_t*)(0xa04ca4) = ba; //ba debug_reset_hook

	inval_I_cache(0xa04ca4, 1);

	reset_func_idx = 0;
}

void debug_reset_handler()
{
	if (reset_func_idx != 0) {
		for (int i = 0; i < reset_func_idx; i++) {
			void (*func)() = reset_func[i];
			func();
		}
	}
}

void debug_run_on_reset(void* func)
{
	if (reset_func_idx > 0x10) {
		printf("Reset func list full\n");
		return;
	}

	//printf("Adding function @ 0x%x to list, num %i\n", (uint32_t)func, reset_func_idx);
	reset_func[reset_func_idx] = func;

	reset_func_idx++;
}