.PHONY: clean all

# Output directory for each submakefiles
OUTPUT := out
export OUTPUT

#
# Some build tools need to be explicitely defined before building. The toolchain
# creates the following platform tools configuration file before it allows the
# toolchain to build.
#
PLATFORM_TOOLS := $(OUTPUT)/platform-tools.mk
export PLATFORM_TOOLS

all: | kernel/$(PLATFORM_TOOLS) user/$(PLATFORM_TOOLS)
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ kernel.bin VERBOSE=$(VERBOSE)

kernel/$(PLATFORM_TOOLS):
	$(MAKE) -C kernel/ $(PLATFORM_TOOLS)

user/$(PLATFORM_TOOLS):
	$(MAKE) -C user/ $(PLATFORM_TOOLS)

clean:
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C user/

run:
	make all
	qemu-system-i386 -m 256 -kernel kernel/kernel.bin
	
debug:
	make all
	qemu-system-i386 -m 256 -kernel kernel/kernel.bin -s -S

.PHONY: doc
doc:
	doxygen

disk:
	   mkdir -p $@

.PHONY: bochs
bochs: all disk
	   @echo "### This target will require root access to mont disk image ! ###"
	   sudo mount -t ext2 -o loop,offset=1048576 disk.img disk/
	   sudo cp kernel/kernel.bin disk/kernel.bin
	   sync
	   sudo umount disk/

.PHONY: cdrom.iso
cdrom.iso: all
	make -C kernel/ cdrom.iso
