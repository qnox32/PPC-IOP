#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

#define DEV_STORE 0x1
#define DEV_LOAD  0x2

#define PATCH_PREFIX  0x1
#define PATCH_POSTFIX 0x2
#define PATCH_REPLACE 0x3

extern uint32_t (*debug_apu_reg_get)(uint32_t reg);
extern uint32_t (*debug_gte_reg_get)(uint32_t reg);
extern uint32_t (*debug_apu_reg_set)(uint32_t reg, uint32_t value);
extern uint32_t (*debug_gte_reg_set)(uint32_t reg, uint32_t value);

uint32_t debug_dcr_get(uint16_t dcr);
uint32_t debug_ppc_reg_get(uint8_t reg);
uint32_t debug_ppc_sreg_get(uint8_t reg);

void debug_dcr_set(uint16_t dcr, uint32_t value);
void debug_ppc_reg_set(uint8_t reg, uint32_t value);
void debug_ppc_sreg_set(uint8_t sreg, uint32_t value);

//patch_type: PATCH_PREFIX, PATCH_POSTFIX, PATCH_REPLACE
void debug_patch_dev_load(uint8_t patch_type, uint32_t addr, uint32_t len, void* func_ptr);
void debug_patch_dev_store(uint8_t patch_type, uint32_t addr, uint32_t len, void* func_ptr);

//place branch after reset to debug_reset_handler()
void debug_hook_reset();
void debug_reset_handler();
void debug_run_on_reset(void* func);

#endif