BUILD_SCRIPTS := $(abspath $(CURDIR)/buildscripts)

all::
	$(MAKE) -C patch
	$(MAKE) -C patch_loader

clean::
	$(MAKE) -C patch clean
	$(MAKE) -C patch_loader clean
