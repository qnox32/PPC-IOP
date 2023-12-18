#include <stdint.h>

#include "ppc_mon.h"
#include "common.h"
#include "debug.h"

static char rx_buf[512];
static int rx_idx;
static int readback;

static int serial_hex_to_int(char *str)
{
	int rv;
	int value = 0;

	while ((*str != ' ') && (*str != '\r')) {

		if (*str >= '0' && *str <= '9') {
			rv = (*str - '0');
		} else if (*str >= 'A' && *str <= 'F') {
			rv = (*str - 'A' + 10);
		} else if (*str >= 'a' && *str <= 'f') {
			rv = (*str - 'a' + 10);
		} else {
			rv = -1;
		}

		value = value * 16 + rv;

		str++;
	}
	return value;
}

static uint32_t serial_get_argv(char *str, int arg)
{
	uint32_t argc = 0;
	uint32_t value = 0;

	while (*str != '\r')
	{
		if (argc == arg) {
			//handle 0x prefix
			if (*str == '0' && *(str+1) == 'x')
				value = serial_hex_to_int(str+2);
			else
				value = serial_hex_to_int(str);
			break;
		}

		if (*str == ' ') {
			argc++;
		}
		
		str++;
	}

	return value;
}

static void ppc_mon_print_help()
{
    printf("PPC-MON v0.1\n");
    printf("\n");
    printf("Memory Commands:\n");
    printf("mrb [addr] = read byte from addr\n");
    printf("mrh [addr] = read halfword from addr\n");
    printf("mrw [addr] = read word from addr\n");
    printf("\n");
    printf("mwb [addr] [value] = write byte value to addr \n");
    printf("mwh [addr] [value] = write halfword value to addr\n");
    printf("mww [addr] [value] = write word value to addr\n");
    printf("\n");
    printf("Register Commands:\n");
    printf("rrd [dcr] = read value from DCR\n");
    printf("rrr [reg] = read value from PPC GP reg (not implemented yet)\n");
    printf("rrs [reg] = read value from PPC SP reg (not implemented yet)\n");
    printf("rra [reg] = read value from APU register file 1 (MIPS + internal)\n");
    printf("rrg [reg] = read value from APU register file 2 (GTE + internal)\n");
    printf("\n");
    printf("rwd [dcr] [value] = write value to DCR\n");
    printf("rwr [reg] [value] = write value to PPC GP reg (not implemented yet)\n");
    printf("rws [reg] [value] = write value to PPC SP reg (not implemented yet)\n");
    printf("rwa [reg] [value] = write value to APU register file 1 (MIPS + internal)\n");
    printf("rwg [reg] [value] = write value to APU register file 2 (GTE + internal)\n");
    printf("\n");
    printf("Dumping Commands:\n");
    printf("dmb [addr] [len] = dump bytes from addr to (addr + len)\n");
    printf("dmh [addr] [len] = dump halfwords from addr to (addr + len)\n");
    printf("dmw [addr] [len] = dump words from addr to (addr + len)\n");
    printf("\n");

}

static void ppc_mon_rx()
{
    char c;

	//Exec this function every 2000 MIPS cycles
	add_event(2000, &ppc_mon_rx, 0);

    //While there is data to read in the RX FIFO
	while (*(uint8_t*)(0x01000205) & 1 != 0) {
		c = *(char*)(0x01000200);
		
        printf("%c", c); //echo back so I can see what I'm typing

		if (rx_idx < 512) {
			rx_buf[rx_idx] = c;
			rx_idx++;
		} else {
			printf("buffer full, command discarded\n");	
			rx_idx = 0;
		}

		//Return
		if (c == '\r') {
			printf("\r\n"); //avoid printing over typed command
            rx_idx=0;

            //Memory Commands
			if (rx_buf[0] == 'm') {
				uint32_t addr = serial_get_argv(&rx_buf, 1);
				if (valid_addr(addr) != 1) {
					printf("Address 0x%x invalid\n", addr);
					printf("\r\n>");
					return;
				}
				
				//Read
				if (rx_buf[1] == 'r') {
					uint32_t value = 0;

					if (rx_buf[2] == 'b')
						value = *(uint8_t*)addr;
					
					if (rx_buf[2] == 'h') {
						if ((addr & 1) != 0) {
							printf("addr not halfword aligned\n");
							return;
						}
						value = *(uint16_t*)addr;
					}

					if (rx_buf[2] == 'w') {
						if ((addr & 3) != 0) {
							printf("addr not word aligned\n");
							return;
						}
						value = *(uint32_t*)addr;
					}

					printf("0x%x: 0x%.8x\n", addr, value);
				}

				//Write
				if (rx_buf[1] == 'w') {
					uint32_t value = serial_get_argv(&rx_buf, 2);

					if (rx_buf[2] == 'b') {
						*(uint8_t*)addr = value;
						if (readback)
                            printf("Mem 0x%x: wrote: 0x%.8x, readback: 0x%.8x\n", addr, value, *(uint8_t*)addr);
                        else
                            printf("Mem 0x%x: wrote: 0x%.8x\n", addr, value);
                    }
					
					if (rx_buf[2] == 'h') {
						if ((addr & 1) != 0) {
							printf("addr not halfword aligned\n");
							return;
						}
						*(uint16_t*)addr = value;
                        if (readback)
						    printf("Mem 0x%x: wrote: 0x%.8x, readback: 0x%.8x\n", addr, value, *(uint16_t*)addr);
                        else
                            printf("Mem 0x%x: wrote: 0x%.8x\n", addr, value);
                    }
					
					if (rx_buf[2] == 'w') {
						if ((addr & 3) != 0) {
							printf("addr not word aligned\n");
							return;
						}
						*(uint32_t*)addr = value;
                        if (readback)
						    printf("Mem 0x%x: wrote: 0x%.8x, readback: 0x%.8x\n", addr, value, *(uint32_t*)addr);
                        else
                            printf("Mem 0x%x: wrote: 0x%.8x\n", addr, value);
					}
				}
			}

			//Register Commands
			if (rx_buf[0] == 'r') {

				uint32_t reg = serial_get_argv(&rx_buf, 1);

				//Read
				if (rx_buf[1] == 'r') {
					
					//DCR
					if (rx_buf[2] == 'd') {
						printf("DCR 0x%x: 0x%.8x\n", reg, debug_dcr_get(reg));
					}
					
					//TODO: PPC GP
					if (rx_buf[2] == 'r') {}

					//TODO: PPC Spec
					if (rx_buf[2] == 's') {}

					//APU Reg file 1
					if (rx_buf[2] == 'a') {
						printf("APU 0x%x: 0x%.8x\n", reg, debug_apu_reg_get(reg));
					}

					//APU Reg file 2
					if (rx_buf[2] == 'g') {
						printf("GTE 0x%x: 0x%.8x\n", reg, debug_gte_reg_get(reg));
					}

				}

				//Write
				if (rx_buf[1] == 'w') {
					
					uint32_t value = serial_get_argv(&rx_buf, 2);
					//DCR
					if (rx_buf[2] == 'd') {
						debug_dcr_set(reg, value);
                        if (readback)
						    printf("DCR 0x%x: wrote: 0x%.8x, readback: 0x%.8x\n", reg, value, debug_dcr_get(reg));
                        else
                            printf("DCR 0x%x: wrote: 0x%.8x\n", reg, value);

                    }
					
					//TODO: PPC GP
					if (rx_buf[2] == 'r') {}

					//TODO: PPC Spec
					if (rx_buf[2] == 's') {}

					//APU Reg file 1
					if (rx_buf[2] == 'a') {
						debug_apu_reg_set(reg, value);
                        if (readback)
						    printf("APU 0x%x: wrote: 0x%.8x, readback: 0x%.8x\n", reg, value, debug_apu_reg_get(reg));
                        else
                            printf("APU 0x%x: wrote: 0x%.8x\n", reg, value);
					}

					//APU Reg file 2
					if (rx_buf[2] == 'g') {
						debug_gte_reg_set(reg, value);
                        if (readback)
						    printf("GTE 0x%x: wrote: 0x%.8x, readback: 0x%.8x\n", reg, value, debug_gte_reg_get(reg));
                        else
                            printf("GTE 0x%x: wrote: 0x%.8x\n", reg, value);
					}
				}
			}

			//Dumping Commands
			if (rx_buf[0] == 'd') {
				
				//Memory
				if (rx_buf[1] = 'm') {
					
					uint32_t addr = serial_get_argv(&rx_buf, 1);
					uint32_t len = serial_get_argv(&rx_buf, 2);
					
					if (!valid_addr(addr) && !valid_addr(addr+len)) {
						printf("Address range: 0x%x - 0x%x is not valid\n", addr, addr + len);
						printf("\r\n>");
						return;
					}

					if (rx_buf[2] == 'b') {
						for (int i = 0; i < len; i++) {
							printf("0x%.8x: 0x%.2x\n", (addr+i), *(uint8_t*)(addr+i));
						}
					}

					if (rx_buf[2] == 'h') {
						if ((addr & 1) != 0) {
							printf("addr not halfword aligned\n");
							printf("\r\n>");
							return;
						}
						for (int i = 0; i < len*2; i+=2) {
							printf("0x%.8x: 0x%.4x\n", (addr+i), *(uint16_t*)(addr+i));
						}
					}

					if (rx_buf[2] == 'w') {
						if ((addr & 3) != 0) {
							printf("addr not word aligned\n");
							printf("\r\n>");
							return;
						}
						for (int i = 0; i < len*4; i+=4) {
							printf("0x%.8x: 0x%.8x\n", (addr+i), *(uint32_t*)(addr+i));
						}
					}
				}
			}

			if (rx_buf[0] == 'h' && rx_buf[1] == 'e' && rx_buf[2] == 'l' && rx_buf[3] == 'p') {
				ppc_mon_print_help();
			}

			printf("\r\n>");
		}

        //Clear on ESC
		if (c == 27) {
			rx_idx = 0;
			printf("\r\n>");
		}
	}
}

void ppc_mon_start()
{
	rx_idx = 0;

	//automatically read back any memory or register written to. On by default
    readback = 1;

	//preserves event through mode resets (PS2 <-> PS1)
	debug_run_on_reset(&ppc_mon_rx); 

	printf("\nWelcome to PPC-MON v0.1\n");
	printf(">");

	ppc_mon_rx();
}