# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.

EE_CC_VERSION := $(shell $(EE_CC) -dumpversion)

EE_OBJS_DIR ?= obj/
EE_SRC_DIR ?= src/
EE_INC_DIR ?= include/
EE_SAMPLE_DIR ?= samples/

EE_INCS := -I$(EE_SRC_DIR) -I$(EE_SRC_DIR)include -I$(EE_INC_DIR) -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include $(EE_INCS)

# Optimization compiler flags
EE_OPTFLAGS ?= -O2 

# Warning compiler flags
EE_WARNFLAGS ?= -Wall

# These flags will generate LTO and non-LTO code in the same object file,
# allowing the choice of using LTO or not in the final linked binary.
#EE_FATLTOFLAGS ?= -flto -ffat-lto-objects

# C compiler flags
EE_CFLAGS := -D_EE -G0 $(EE_OPTFLAGS) $(EE_WARNFLAGS) $(EE_INCS) $(EE_CFLAGS)

ifeq ($(DEBUG),1)
EE_CFLAGS += -DDEBUG
endif
# C++ compiler flags
EE_CXXFLAGS := -D_EE -G0 -$(EE_OPTFLAGS) $(EE_WARNFLAGS) $(EE_INCS) $(EE_CXXFLAGS)

# Linker flags
# EE_LDFLAGS := $(EE_LDFLAGS)

# Assembler flags
EE_ASFLAGS := $(EE_ASFLAGS)

EE_SAMPLES := $(EE_SAMPLES:%=$(EE_SAMPLE_DIR)%)

EE_OBJS := $(EE_OBJS:%=$(EE_OBJS_DIR)%)

EE_LINKFILE ?= $(PS2SDK)/ee/startup/linkfile

EE_BIN_MAPFILE ?= $(shell basename $(CURDIR)).map

# Externally defined variables: EE_BIN, EE_OBJS, EE_LIB

# These macros can be used to simplify certain build rules.
EE_C_COMPILE = $(EE_CC) -MD $(EE_CFLAGS)
EE_CXX_COMPILE = $(EE_CXX) $(EE_CXXFLAGS)

# Command for ensuring the output directory for the rule exists.
DIR_GUARD = @$(MKDIR) -p $(@D)

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.c
	$(DIR_GUARD)
	$(EE_C_COMPILE) $(EE_FATLTOFLAGS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.cpp
	$(DIR_GUARD)
	$(EE_CXX_COMPILE) $(EE_FATLTOFLAGS) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.S
	$(DIR_GUARD)
	$(EE_C_COMPILE) -c $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_SRC_DIR)%.s
	$(DIR_GUARD)
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

$(EE_OBJS_DIR)%.o: $(EE_ASM_DIR)%.s
	$(DIR_GUARD)
	$(EE_AS) $(EE_ASFLAGS) $< -o $@

$(EE_BIN): $(EE_OBJS) $(EE_LIB_ARCHIVES) $(EE_ADDITIONAL_DEPS) 
	$(DIR_GUARD)
	$(EE_CC) -T$(EE_LINKFILE) -Wl,-Map,$(EE_OBJS_DIR)$(EE_BIN_MAPFILE) $(EE_OPTFLAGS) \
		-o $(EE_BIN) $(EE_OBJS) $(EE_LDFLAGS) $(EE_LIB_ARCHIVES) $(EE_LIBS)
ifneq ($(EE_STRIP_FLAGS),0)
	$(EE_STRIP) $(EE_STRIP_FLAGS) $(EE_BIN)
endif

$(EE_LIB): $(EE_OBJS) $(EE_LIB:%.a=%.erl)
	$(DIR_GUARD)
	$(EE_AR) cru $(EE_LIB) $(EE_OBJS)

$(EE_LIB:%.a=%.erl): $(EE_OBJS)
	$(DIR_GUARD)
	$(EE_CC) -nostdlib $(EE_NO_CRT) -Wl,-r -Wl,-d -o $(EE_LIB:%.a=%.erl) $(EE_OBJS)
	$(EE_STRIP) --strip-unneeded -R .mdebug.eabi64 -R .reginfo -R .comment $(EE_LIB:%.a=%.erl)

# Header depends:
-include $(EE_OBJS:.o=.d)