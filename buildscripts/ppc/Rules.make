# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

PPC_CC_VERSION := $(shell $(PPC_CC) -dumpversion)

PPC_OBJS_DIR ?= obj/
PPC_SRC_DIR ?= src/
PPC_INC_DIR ?= include/


PPC_INCS := -I$(PPC_SRC_DIR) -I$(PPC_INC_DIR)

# -fno-delete-null-pointer-checks
# Optimization:
PPC_OPTFLAGS ?= -O1 -fno-delete-null-pointer-checks

# Warnings:
PPC_WARNFLAGS ?= -Wall 

# -fno-asynchronous-unwind-tables prevents the generation of .eh_frame

PPC_CFLAGS := -mcpu=440 -fno-asynchronous-unwind-tables -ffreestanding -fno-builtin -nostartfiles -nostdlib -G0 $(PPC_OPTFLAGS) $(PPC_WARNFLAGS) $(PPC_INCS) $(PPC_CFLAGS)
ifeq ($(DEBUG),1)
PPC_CFLAGS += -DDEBUG
endif

# Additional C compiler flags for GCC >=v5.3.0
# -msoft-float is to "remind" GCC/Binutils that the soft-float ABI is to be used. This is due to a bug, which
#   results in the ABI not being passed correctly to binutils and PPC-as defaults to the hard-float ABI instead.
# -mno-explicit-relocs is required to work around the fact that GCC is now known to
#   output multiple LO relocs after one HI reloc (which the PPC kernel cannot deal with).
# -fno-toplevel-reorder (for PPC import and export tables only) disables toplevel reordering by GCC v4.2 and later.
#   Without it, the import and export tables can be broken apart by GCC's optimizations.
ifneq ($(PPC_CC_VERSION),3.2.2)
ifneq ($(PPC_CC_VERSION),3.2.3)
PPC_CFLAGS += -msoft-float
#PPC_IETABLE_CFLAGS := -fno-toplevel-reorder
endif
endif

# Assembler flags
PPC_ASFLAGS := $(ASFLAGS_TARGET) -G0 $(PPC_ASFLAGS)

PPC_OBJS := $(PPC_OBJS:%=$(PPC_OBJS_DIR)%)

# Externally defined variables: PPC_BIN, PPC_OBJS, PPC_LIB

# These macros can be used to simplify certain build rules.
#PPC_C_COMPILE = $(PPC_CC) -MD $(PPC_CFLAGS)
PPC_C_COMPILE = $(PPC_CC) $(PPC_CFLAGS)

# Depends
-include $(PPC_OBJS:.o=.d)

# Command for ensuring the output directory for the rule exists.
DIR_GUARD = @$(MKDIR) -p $(@D)

# Project build rules:
$(PPC_OBJS_DIR)%.o: $(PPC_SRC_DIR)%.c
	$(DIR_GUARD)
	$(PPC_C_COMPILE) -c $< -o $@

$(PPC_OBJS_DIR)%.o: $(PPC_SRC_DIR)%.S
	$(DIR_GUARD)
	$(PPC_C_COMPILE) -c $< -o $@

$(PPC_OBJS_DIR)%.o: $(PPC_SRC_DIR)%.s
	$(DIR_GUARD)
	$(PPC_AS) $(PPC_ASFLAGS) $< -o $@

.INTERMEDIATE: $(PPC_OBJS_DIR)build-imports.c $(PPC_OBJS_DIR)build-exports.c

$(PPC_OBJS_DIR)template-imports.h:
	$(DIR_GUARD)
	$(ECHO) "#include \"irx_imports.h\"" > $@

# Rules to build imports.lst.
$(PPC_OBJS_DIR)build-imports.c: $(PPC_OBJS_DIR)template-imports.h $(PPC_SRC_DIR)imports.lst
	$(DIR_GUARD)
	cat $^ > $@

$(PPC_OBJS_DIR)imports.o: $(PPC_OBJS_DIR)build-imports.c
	$(DIR_GUARD)
	$(PPC_C_COMPILE) $(PPC_IETABLE_CFLAGS) -c $< -o $@

$(PPC_OBJS_DIR)template-exports.h:
	$(DIR_GUARD)
	$(ECHO) "#include \"irx.h\"" > $@

# Rules to build exports.tab.
$(PPC_OBJS_DIR)build-exports.c: $(PPC_OBJS_DIR)template-exports.h $(PPC_SRC_DIR)exports.tab
	$(DIR_GUARD)
	cat $^ > $@

$(PPC_OBJS_DIR)exports.o: $(PPC_OBJS_DIR)build-exports.c
	$(DIR_GUARD)
	$(PPC_C_COMPILE) $(PPC_IETABLE_CFLAGS) -c $< -o $@

$(PPC_BIN): $(PPC_OBJS) $(PPC_LIB_ARCHIVES) $(PPC_ADDITIONAL_DEPS)
	$(DIR_GUARD)
	$(PPC_C_COMPILE) $(PPC_OPTFLAGS) -o $(PPC_BIN) $(PPC_OBJS) $(PPC_LDFLAGS) $(PPC_LIB_ARCHIVES) $(PPC_LIBS)
	$(PPC_STRIP) --strip-unneeded -R .comment -R .eh_frame $(PPC_BIN)

$(PPC_LIB): $(PPC_OBJS)
	$(DIR_GUARD)
	$(PPC_AR) cru $@ $(PPC_OBJS)

# Header depends:
-include $(PPC_OBJS:.o=.d)