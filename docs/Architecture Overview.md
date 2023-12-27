## Preface
This document aims to provide a high-level overview of the PPC-IOP ASIC and is currently a work in progress. Some of the information here may be inaccurate or incomplete.

## Overview
The PPC-IOP ASIC, present in SCHP-75K and newer PS2 models, features a hybrid hardware and software approach to 
emulating the MIPS R3000A processor found in the PS1 and 70K or older PS2 models. The hardware portion of this emulation
comes in the form of an Auxiliary Processing Unit or APU attached to a PPC 440 core clocked at 440MHz. 
The software portion comes in the form of the "DECKARD" emulation software.

## Hardware
The Auxiliary Processing Unit (APU) consists of a MIPS instruction decoder, GTE, and two register files. It is 
capable of independently performing arithmetic and logic operations, as well as moving data within the same register file.
However, it lacks the ability to fetch its own instructions or directly access memory in any way. All fetch, load, store,
branch, division, and multiplication operations require assistance from PPC code. To facilitate data transfer between 
the APU and PPC core, a set of custom instructions is utilized (see APU_opcodes.txt). The APU operates at the same 
clock speed as the PPC core (440MHz), and the timing of the original MIPS-IOP / PS1 is emulated through a software 
timing/event system. For every 1 MIPS instruction to execute, x number of PPC instructions must also be executed. 
Consequently, there are some operations, that under certain conditions may take longer to complete on the PPC-IOP 
due to the length of the code path the emulator must take.

### Geometry Transformation Engine (GTE)
The GTE is also a part of the APU. In addition to the normal 0-63 GTE registers, there's an instruction register, 
execution status register, and cycle count register. (see APU_registers.txt) Instructions and data are copied from the
MIPS register file to the GTE register file via PPC code. Some instructions appear to be modified by the PPC prior 
to being copied over. Currently unknown how and why. The cycle count register contains the number of cycles the last
executed GTE instruction would take on a real PS1/MIPS-IOP, this value is retrieved and then subtracted from the 
current MIPS cycle count register. This suggests the GTE may be running at the same speed as the PPC core, although this has not been confirmed.

### Memory
The PPC-IOP has 4MBs of SDRAM. The lower 2MBs are reserved for normal MIPS-IOP usage. The upper 2MBs are used by 
the "DECKARD" emulator. The emulator code and data only take up 65KB. The majority of this space is occupied by two 
lookup tables, each 262KB in size. These LUTs contain function pointers to load and store functions for devices/addresses
in the 0x1f801000 - 0x1f900fff range. It's possible this approach was chosen because it's faster than doing multiple
compares on an address and then branching to the appropriate function. Since nothing exists between 0x1F808600 - 0x1F900000,
there is a significant amount of unused space in each table. The patch provided makes use of this unused space (see patch_info.txt).
Additionally, there are no sanity checks on reads or writes within the A00000-BFFFFF address range, allowing any 
IOP module running on a PPC-IOP PS2 to read and write to this region. B00000 - BFFFFF are mapped as non executable.

Memory Layout:
| Virtual Address | Physical Address|     Function     |         Size         |
|-----------------|-----------------|------------------|----------------------|
| A00000 - A0E1A4 | A00000 - A0E1A4 | DECKARD ELF code | (0xE1A4)  57.764 KB  |
| A0E1A8 - A0FFFF | A0E1A8 - A0FFFF | DECKARD ELF data | (0x1E58)  7.768 KB   |
| A10000 - A7FFFF | A10000 - A7FFFF | Unused           | (0x70000) 458.752 KB |
| A80000 - ABFFFF | A80000 - ABFFFF | Load LUT         | (0x40000) 262.143 KB |
| AC0000 - AFFFFF | AC0000 - AFFFFF | Store LUT        | (0x40000) 262.143 KB |
| B00000 - B3FFFF | Unmapped        | ?                | (0x40000) 262.144 KB |
| B40000 - B403FF | B40000 - B403FF | PS1DRV related   | (0x400)   1 KB       |
| B40400 - B407FF | B40400 - B407FF | PS1DRV related   | (0x400)   1 KB       |
| B40800 - B40BFF | Unmapped        | ?                | (0x400)   1 KB       |
| B40C00 - B40FFF | FFFC00 - FFFFFF | ?                | (0x400)   1 KB       |
| B41000 - BDE7FF | Unmapped        | ?                | (0x9D800) 645.120 KB |
| BDE800 - BDEBFF | B00000 - B003FF | Cache (R/O)      | (0x400)   1 KB       |
| BDEC00 - BDEFFF | B00400 - B007FF | Cache (R/O)      | (0x400)   1 KB       |
| BDF000 - BDFFFF | B7F000 - B7FFFF | PPC Stack (R/O)  | (0x1000)  4 KB       |
| BE0000 - BE0FFF | B60000 - B60FFF | PPC Variables    | (0x1000)  4 KB       |
| BE1000 - BFEFFF | Unmapped        | Unmapped         | (0x1E000) 122.880KB  |
| BFF000 - BFFFFF | B7F000 - B7FFFF | PPC Stack (R/W)  | (0x1000)  4 KB       |

### Cache
There does not appear to be any discrete i-cache or d-cache for the APU. Since both MIPS instructions and data are 
handled by PPC code, the 32KB d-cache functions as d & i cache for MIPS. Scratchpad is memory mapped for emulation 
purposes. PARAM_MIPS_DCACHE_ON is used to swap between two sets of TLB entries that map the lower 2MBs. 
One set has caching enabled, while the other has caching disabled.

### IOP Devices
Several devices have special handlers that will patch values if specific conditions are met. The "IOP Device Quirks" 
folder contains the devices that have been documented thus far.

## Software
The DECKARD emulation software is responsible for fetching instructions, loading, and storing data for the APU, moving 
data between APU register files (MIPS <-> GTE), timing emulation, patching values and more. For the sake of simplicity
the software can be broken down into two major components: the event + interrupt handler and the main emulation loop. 
The event + interrupt handler exists at the top level and controls how many MIPS cycles the main emulation loop will 
process at a time.

The main emulation loop is essentially:
Fetch MIPS instruction -> Copy to APU using custom instructions -> Get instruction type (logic, load, store, etc) and operand address (if applicable) from APU -> Perform operations required for instruction type -> Repeat until MIPS cycle count register reaches zero or interrupt occurs.

Diagram:
![main_emu_diag](https://github.com/qnox32/PPC-IOP/assets/123997012/048f8a4e-58a9-47db-8b91-00208bcf9b32)

When the MIPS cycle count register reaches zero or an interrupt occurs, and the subsequent instruction is not in a delay slot, the APU will set the instruction type flag to zero when the following instruction is passed to it. This causes execution to branch out of the main emulation loop and into the event / interrupt handler. However, if these conditions occur when the subsequent instruction IS in a delay slot, the loop continues until a non-delay slot instruction is encountered. If the branch was triggered by the cycle count reaching zero, the previously scheduled event is called. The event handler then determines the next event to run and sets the MIPS cycle count register accordingly upon re-entering the main emulation. ie: next event -> event trigger time = 2000 MIPS cycles -> emulation_loop_run(2000 cycles). There is always one event scheduled to run which is the timing control event. The timing event is responsible for approximating the speed of the original MIPS-IOP or PS1 depending on the mode. (TODO: more on timing control)

### Interrupts
Interrupts are partially emulated. Values written to I_MASK are always AND'd with 0xf900008c. It's assumed that only values within the 0xf900008c mask can be written to the INTC from the PPC-IOP. The INTC feeds interrupts into the APU status register 0x4 bit 16. When this bit is set the APU will not decode/execute any instruction it's given, will not increment PC, will not decrement cycle counter, and will set the instruction type flag to zero when given the next instruction. This will cause the PPC to break out of the main emulation loop to handle the interrupt. The MIPS PC register only increments on its own, requiring the PPC to manually set it to the interrupt handler address. Emulated copies of I_MASK, I_STAT, and I_CTRL are used to swap bit 13 and 26 (Dev 9) among other things that are currently unknown. 

### XPARAMS
XPARAMs are used to control various properties of the emulator and improve compatibility. Normally XPARAMs are provided by the game disc and loaded into memory using the XPARAM IOP module. However, they exist within a struct in memory and can be manipulated directly. See XPARAMs.txt for more details.
