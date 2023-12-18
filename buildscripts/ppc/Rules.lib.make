# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

PPC_LIB_DIR ?= lib/

PPC_LIB ?= lib$(shell basename $(CURDIR)).a
PPC_LIB := $(PPC_LIB:%=$(PPC_LIB_DIR)%)

all:: $(PPC_LIB)

clean::
	rm -f -r $(PPC_OBJS_DIR) $(PPC_LIB_DIR)
