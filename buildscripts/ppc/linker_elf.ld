/* "Bare" static ELF */

ENTRY(_start);

SECTIONS {

	. = 0xA14004;

	.text : {
		*(.text_entry)
		*(.text)
	}

	.rodata ALIGN(4) : {
		*(.rodata)
	}

	.data ALIGN(4) : {
		*(.data)
	}

	.sdata ALIGN(4) : {
		*(.sdata) 
	}

	.sdata2 ALIGN(4) : {
		*(.sdata2) 
	}

	.bss ALIGN(4) : {
		*(COMMON)
		*(.bss)
	}

	.sbss ALIGN(4) : {
		*(.sbss) 
	}

	.comment ALIGN(4) : {
		*(.comment) 
	}

	PROVIDE (_end = .);
}
