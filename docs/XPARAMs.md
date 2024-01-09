# PPC-IOP XPARAMs
>v0.1 2023  
> by nox

The XPARAM IOP module writes PARAM values to `0xFFFE01a0` and `0xFFFE01a4`.  
In the PPC store handler for these addresses copy the PARAM value to a 
struct in memory @ `0xbe09e0` - `0xbe0a24`.  
From there the values are used
by their respective functions. The XPARAM struct can written to directly
bypassing the need to use the XPARAM IOP module.

- `0xFFFE01a0` = index
- `0xFFFE01a4` = value

## XPARAM Struct Addr: `0xbe09e0` - `0xbe0a24`

| offset | name
| ------ | -------------------------------
| `0x0`  | `PARAM_MDEC_DELAY_CYCLE`
| `0x4`  | `PARAM_SPU_INT_DELAY_LIMIT`
| `0x8`  | `PARAM_SPU_INT_DELAY_PPC_COEFF`
| `0xC`  | `PARAM_SPU2_INT_DELAY_LIMIT`
| `0x10` | `PARAM_SPU2_INT_DELAY_PPC_COEFF`
| `0x14` | `PARAM_DMAC_CH10_INT_DELAY`
| `0x18` | `PARAM_CPU_DELAY`
| `0x1C` | `PARAM_SPU_DMA_WAIT_LIMIT`
| `0x20` | `PARAM_GPU_DMA_WAIT_LIMIT`
| `0x24` | `PARAM_DMAC_CH10_INT_DELAY_DPC`
| `0x28` | `PARAM_CPU_DELAY_DPC`
| `0x2c` | `PARAM_USB_DELAYED_INT_ENABLE`
| `0x30` | `PARAM_TIMER_LOAD_DELAY`
| `0x34` | `PARAM_SIO0_DTR_SCK_DELAY`
| `0x38` | `PARAM_SIO0_DSR_SCK_DELAY_C`
| `0x3c` | `PARAM_SIO0_DSR_SCK_DELAY_M`
| `0x40` | `PARAM_MIPS_DCACHE_ON`
| `0x44` | `PARAM_CACHE_FLASH_CHANNELS`

#### `PARAM_CPU_DELAY` is used to control timing:
```
(ppc_clk_delta * mips_clk_factor) / PARAM_CPU_DELAY or PARAM_CPU_DELAY_DPC
```

#### `PARAM_TIMER_LOAD_DELAY` Controls load delay when reading from timer0 in ps1 mode

#### `PARAM_SIO0_D*` appear to be multiples of the CPU clock period (2.27ns @ 440MHz).
- `PARAM_SIO0_DTR_SCK_DELAY`:   `0x229c == 8860 * 2.27ns = 20.1122 uS`
- `PARAM_SIO0_DSR_SCK_DELAY_C`: `0x6ec  == 1772 * 2.27ns = 4.02244 uS`
- `PARAM_SIO0_DSR_SCK_DELAY_M`: `0x6ec  == 1772 * 2.27ns = 4.02244 uS`

#### `PARAM_MIPS_DCACHE_ON`
Controls which set of TLB entries are used for the lower 2MBs of IOP RAM.
- `0` = uncached entries for this region
- `1` = cached entries for this region
