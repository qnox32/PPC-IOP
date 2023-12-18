# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

PPC_LDFLAGS += -T $(BUILD_SCRIPTS)/ppc/linker_elf.ld
PPC_BIN_CNT_SUFFIX =.elf

PPC_BIN_DIR ?= bin/

PPC_BIN ?= $(shell basename $(CURDIR))

PPC_BIN := $(PPC_BIN:%=$(PPC_BIN_DIR)%)$(PPC_BIN_CNT_SUFFIX)

all:: $(PPC_BIN)

clean::
	rm -f -r $(PPC_OBJS_DIR) $(PPC_BIN_DIR)
