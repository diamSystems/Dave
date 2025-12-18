INCLUDE_DIR := include

BOOT_STAGE0 := boot/stage0
BOOT_STAGE1 := boot/stage1
BOOT_INIT := boot/init
BOOT_TOOLS := boot/tools

SYSTEM_ROOT := system
KERNEL_CORE := $(SYSTEM_ROOT)/core
KERNEL_STORAGE := $(SYSTEM_ROOT)/storage
KERNEL_DEVICES := $(SYSTEM_ROOT)/devices
KERNEL_MEMORY := $(SYSTEM_ROOT)/memory
KERNEL_RUNTIME := $(SYSTEM_ROOT)/runtime
KERNEL_SYNC := $(SYSTEM_ROOT)/sync
KERNEL_PLATFORM_X86 := $(SYSTEM_ROOT)/platform/x86

USER_BIN_DIR := user/bin
USER_LIB_DIR := user/lib
USER_TEST_DIR := user/tests

DEV_MKFS_DIR := devtools/mkfs
DEV_DEBUG_DIR := devtools/debug

BUILD_DIR := build
BUILD_BIN_DIR := $(BUILD_DIR)/bin
BUILD_USER_DIR := $(BUILD_BIN_DIR)/user
BUILD_IMG_DIR := $(BUILD_DIR)/image
BUILD_TOOL_DIR := $(BUILD_DIR)/tools
BUILD_ARTIFACT_DIR := $(BUILD_DIR)/artifacts
FS_STAGING_DIR := $(BUILD_DIR)/rootfs

BUILD_DIRS := $(BUILD_DIR) $(BUILD_BIN_DIR) $(BUILD_USER_DIR) $(BUILD_IMG_DIR) $(BUILD_TOOL_DIR) $(BUILD_ARTIFACT_DIR) $(FS_STAGING_DIR)

MKFS_BIN := $(BUILD_TOOL_DIR)/mkfs
BOOTBLOCK_BIN := $(BUILD_BIN_DIR)/bootblock
ENTRYOTHER_BIN := $(BUILD_BIN_DIR)/entryother
INITCODE_BIN := $(BUILD_BIN_DIR)/initcode
ENTRYOTHER_LINK := $(BUILD_ARTIFACT_DIR)/entryother
INITCODE_LINK := $(BUILD_ARTIFACT_DIR)/initcode
ENTRYOTHER_SYM_PREFIX := _binary_build_artifacts_entryother
INITCODE_SYM_PREFIX := _binary_build_artifacts_initcode
BIN_ALIAS_FLAGS := \
	--defsym _binary_entryother_start=$(ENTRYOTHER_SYM_PREFIX)_start \
	--defsym _binary_entryother_end=$(ENTRYOTHER_SYM_PREFIX)_end \
	--defsym _binary_entryother_size=$(ENTRYOTHER_SYM_PREFIX)_size \
	--defsym _binary_initcode_start=$(INITCODE_SYM_PREFIX)_start \
	--defsym _binary_initcode_end=$(INITCODE_SYM_PREFIX)_end \
	--defsym _binary_initcode_size=$(INITCODE_SYM_PREFIX)_size
ENTRY_OBJ := $(BUILD_ARTIFACT_DIR)/entry.o
BOOTASM_OBJ := $(BUILD_ARTIFACT_DIR)/bootasm.o
BOOTMAIN_OBJ := $(BUILD_ARTIFACT_DIR)/bootmain.o
BOOTBLOCK_OBJ := $(BUILD_ARTIFACT_DIR)/bootblock.o
ENTRYOTHER_OBJ := $(BUILD_ARTIFACT_DIR)/entryother.o
INITCODE_OBJ := $(BUILD_ARTIFACT_DIR)/initcode.o
INITCODE_OUT := $(BUILD_ARTIFACT_DIR)/initcode.out
KERNEL_BIN := $(BUILD_BIN_DIR)/kernel
KERNELMEMFS_BIN := $(BUILD_BIN_DIR)/kernelmemfs
BOOTBLOCK_ASM := $(BUILD_ARTIFACT_DIR)/bootblock.asm
ENTRYOTHER_ASM := $(BUILD_ARTIFACT_DIR)/entryother.asm
INITCODE_ASM := $(BUILD_ARTIFACT_DIR)/initcode.asm
KERNEL_ASM := $(BUILD_ARTIFACT_DIR)/kernel.asm
KERNEL_SYM := $(BUILD_ARTIFACT_DIR)/kernel.sym
KERNELMEMFS_ASM := $(BUILD_ARTIFACT_DIR)/kernelmemfs.asm
KERNELMEMFS_SYM := $(BUILD_ARTIFACT_DIR)/kernelmemfs.sym
FS_IMG := $(BUILD_IMG_DIR)/fs.img
XV6_IMG := $(BUILD_IMG_DIR)/xv6.img
XV6_MEMFS_IMG := $(BUILD_IMG_DIR)/xv6memfs.img

KERNEL_OBJS = \
	$(KERNEL_STORAGE)/bio.o\
	$(KERNEL_DEVICES)/console.o\
	$(KERNEL_CORE)/exec.o\
	$(KERNEL_CORE)/file.o\
	$(KERNEL_STORAGE)/fs.o\
	$(KERNEL_STORAGE)/ide.o\
	$(KERNEL_PLATFORM_X86)/ioapic.o\
	$(KERNEL_MEMORY)/kalloc.o\
	$(KERNEL_DEVICES)/kbd.o\
	$(KERNEL_PLATFORM_X86)/lapic.o\
	$(KERNEL_STORAGE)/log.o\
	$(KERNEL_CORE)/main.o\
	$(KERNEL_PLATFORM_X86)/mp.o\
	$(KERNEL_PLATFORM_X86)/picirq.o\
	$(KERNEL_CORE)/pipe.o\
	$(KERNEL_CORE)/proc.o\
	$(KERNEL_SYNC)/sleeplock.o\
	$(KERNEL_SYNC)/spinlock.o\
	$(KERNEL_RUNTIME)/string.o\
	$(KERNEL_RUNTIME)/swtch.o\
	$(KERNEL_CORE)/syscall.o\
	$(KERNEL_CORE)/sysfile.o\
	$(KERNEL_CORE)/sysproc.o\
	$(KERNEL_PLATFORM_X86)/trapasm.o\
	$(KERNEL_PLATFORM_X86)/trap.o\
	$(KERNEL_DEVICES)/uart.o\
	$(KERNEL_PLATFORM_X86)/vectors.o\
	$(KERNEL_MEMORY)/vm.o\

OBJS = $(KERNEL_OBJS)

USER_BINS := cat clear echo grep hello init kill ln ls mkdir rm stressfs ted usertests wc zombie
USER_BIN_SRCS := $(addprefix $(USER_BIN_DIR)/,$(addsuffix .c,$(USER_BINS)))
USER_BIN_OBJS := $(USER_BIN_SRCS:.c=.o)

USER_TESTS := forktest
USER_TEST_SRCS := $(addprefix $(USER_TEST_DIR)/,$(addsuffix .c,$(USER_TESTS)))
USER_TEST_OBJS := $(USER_TEST_SRCS:.c=.o)

UPROG_NAMES := $(addprefix _,$(USER_BINS))
UPROGS = $(addprefix $(BUILD_USER_DIR)/,$(UPROG_NAMES)) $(BUILD_USER_DIR)/_forktest
UPROG_STAGING_NAMES := $(addprefix bin/,$(UPROG_NAMES)) bin/_forktest
STAGED_UPROGS = $(addprefix $(FS_STAGING_DIR)/,$(UPROG_STAGING_NAMES))
STAGED_README_NAME := root/README
STAGED_README := $(FS_STAGING_DIR)/$(STAGED_README_NAME)

ULIB = \
	$(USER_LIB_DIR)/ulib.o\
	$(USER_LIB_DIR)/usys.o\
	$(USER_LIB_DIR)/printf.o\
	$(USER_LIB_DIR)/umalloc.o\
	$(USER_LIB_DIR)/structio.o\
	$(USER_LIB_DIR)/modern.o

# Cross-compiling (e.g., on Mac OS X)
# TOOLPREFIX = i386-jos-elf

# Using native tools (e.g., on X86 Linux)
#TOOLPREFIX = 

# Try to infer the correct TOOLPREFIX if not set
ifndef TOOLPREFIX
TOOLPREFIX := $(shell if i386-jos-elf-objdump -i 2>&1 | grep '^elf32-i386$$' >/dev/null 2>&1; \
	then echo 'i386-jos-elf-'; \
	elif objdump -i 2>&1 | grep 'elf32-i386' >/dev/null 2>&1; \
	then echo ''; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find an i386-*-elf version of GCC/binutils." 1>&2; \
	echo "*** Is the directory with i386-jos-elf-gcc in your PATH?" 1>&2; \
	echo "*** If your i386-*-elf toolchain is installed with a command" 1>&2; \
	echo "*** prefix other than 'i386-jos-elf-', set your TOOLPREFIX" 1>&2; \
	echo "*** environment variable to that prefix and run 'make' again." 1>&2; \
	echo "*** To turn off this error, run 'gmake TOOLPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

# If the makefile can't find QEMU, specify its path here
# QEMU = qemu-system-i386

# Try to infer the correct QEMU
ifndef QEMU
QEMU = $(shell if which qemu > /dev/null; \
	then echo qemu; exit; \
	elif which qemu-system-i386 > /dev/null; \
	then echo qemu-system-i386; exit; \
	elif which qemu-system-x86_64 > /dev/null; \
	then echo qemu-system-x86_64; exit; \
	else \
	qemu=/Applications/Q.app/Contents/MacOS/i386-softmmu.app/Contents/MacOS/i386-softmmu; \
	if test -x $$qemu; then echo $$qemu; exit; fi; fi; \
	echo "***" 1>&2; \
	echo "*** Error: Couldn't find a working QEMU executable." 1>&2; \
	echo "*** Is the directory containing the qemu binary in your PATH" 1>&2; \
	echo "*** or have you tried setting the QEMU variable in Makefile?" 1>&2; \
	echo "***" 1>&2; exit 1)
endif

CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gas
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump
CPPFLAGS = -iquote $(INCLUDE_DIR)
CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -fno-omit-frame-pointer $(CPPFLAGS)
ASFLAGS = -m32 -gdwarf-2 -Wa,-divide $(CPPFLAGS)
# FreeBSD ld wants ``elf_i386_fbsd''
LDFLAGS += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector),)
CFLAGS += -fno-stack-protector
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif


$(BUILD_DIRS):
	mkdir -p $@

PHONY_IMAGES := xv6.img xv6memfs.img fs.img

$(PHONY_IMAGES):
	@echo "Use $(BUILD_IMG_DIR) for generated images"

$(XV6_IMG): $(BOOTBLOCK_BIN) $(KERNEL_BIN) | $(BUILD_DIRS)
	dd if=/dev/zero of=$@ count=10000
	dd if=$(BOOTBLOCK_BIN) of=$@ conv=notrunc
	dd if=$(KERNEL_BIN) of=$@ seek=1 conv=notrunc

$(XV6_MEMFS_IMG): $(BOOTBLOCK_BIN) $(KERNELMEMFS_BIN) | $(BUILD_DIRS)
	dd if=/dev/zero of=$@ count=10000
	dd if=$(BOOTBLOCK_BIN) of=$@ conv=notrunc
	dd if=$(KERNELMEMFS_BIN) of=$@ seek=1 conv=notrunc

$(BOOTBLOCK_BIN): $(BOOT_STAGE0)/bootasm.S $(BOOT_STAGE1)/bootmain.c | $(BUILD_DIRS)
	$(CC) $(CFLAGS) -fno-pic -O -nostdinc -I$(INCLUDE_DIR) -c $(BOOT_STAGE1)/bootmain.c -o $(BOOTMAIN_OBJ)
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I$(INCLUDE_DIR) -c $(BOOT_STAGE0)/bootasm.S -o $(BOOTASM_OBJ)
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o $(BOOTBLOCK_OBJ) $(BOOTASM_OBJ) $(BOOTMAIN_OBJ)
	$(OBJDUMP) -S $(BOOTBLOCK_OBJ) > $(BOOTBLOCK_ASM)
	$(OBJCOPY) -S -O binary -j .text $(BOOTBLOCK_OBJ) $(BOOTBLOCK_BIN)
	$(BOOT_TOOLS)/sign.pl $(BOOTBLOCK_BIN)

$(ENTRYOTHER_BIN): $(BOOT_INIT)/entryother.S | $(BUILD_DIRS)
	$(CC) $(CFLAGS) -fno-pic -nostdinc -I$(INCLUDE_DIR) -c $(BOOT_INIT)/entryother.S -o $(ENTRYOTHER_OBJ)
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o $(BUILD_ARTIFACT_DIR)/bootblockother.o $(ENTRYOTHER_OBJ)
	$(OBJCOPY) -S -O binary -j .text $(BUILD_ARTIFACT_DIR)/bootblockother.o $(ENTRYOTHER_BIN)
	cp $(ENTRYOTHER_BIN) $(ENTRYOTHER_LINK)
	$(OBJDUMP) -S $(BUILD_ARTIFACT_DIR)/bootblockother.o > $(ENTRYOTHER_ASM)

$(INITCODE_BIN): $(BOOT_INIT)/initcode.S | $(BUILD_DIRS)
	$(CC) $(CFLAGS) -nostdinc -I$(INCLUDE_DIR) -c $(BOOT_INIT)/initcode.S -o $(INITCODE_OBJ)
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $(INITCODE_OUT) $(INITCODE_OBJ)
	$(OBJCOPY) -S -O binary $(INITCODE_OUT) $(INITCODE_BIN)
	cp $(INITCODE_BIN) $(INITCODE_LINK)
	$(OBJDUMP) -S $(INITCODE_OBJ) > $(INITCODE_ASM)

$(ENTRY_OBJ): $(BOOT_INIT)/entry.S | $(BUILD_DIRS)
	$(CC) $(CFLAGS) -nostdinc -I$(INCLUDE_DIR) -c $(BOOT_INIT)/entry.S -o $(ENTRY_OBJ)

$(KERNEL_BIN): $(OBJS) $(ENTRY_OBJ) $(ENTRYOTHER_BIN) $(INITCODE_BIN) $(BOOT_INIT)/kernel.ld | $(BUILD_DIRS)
	$(LD) $(LDFLAGS) $(BIN_ALIAS_FLAGS) -T $(BOOT_INIT)/kernel.ld -o $(KERNEL_BIN) $(ENTRY_OBJ) $(OBJS) -b binary $(INITCODE_LINK) $(ENTRYOTHER_LINK)
	$(OBJDUMP) -S $(KERNEL_BIN) > $(KERNEL_ASM)
	$(OBJDUMP) -t $(KERNEL_BIN) | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(KERNEL_SYM)

# kernelmemfs is a copy of kernel that maintains the
# disk image in memory instead of writing to a disk.
# This is not so useful for testing persistent storage or
# exploring disk buffering implementations, but it is
# great for testing the kernel on real hardware without
# needing a scratch disk.
MEMFSOBJS = $(filter-out $(KERNEL_STORAGE)/ide.o,$(OBJS)) $(KERNEL_STORAGE)/memide.o
$(KERNELMEMFS_BIN): $(MEMFSOBJS) $(ENTRY_OBJ) $(ENTRYOTHER_BIN) $(INITCODE_BIN) $(BOOT_INIT)/kernel.ld $(FS_IMG) | $(BUILD_DIRS)
	$(LD) $(LDFLAGS) $(BIN_ALIAS_FLAGS) -T $(BOOT_INIT)/kernel.ld -o $(KERNELMEMFS_BIN) $(ENTRY_OBJ) $(MEMFSOBJS) -b binary $(INITCODE_LINK) $(ENTRYOTHER_LINK) $(FS_IMG)
	$(OBJDUMP) -S $(KERNELMEMFS_BIN) > $(KERNELMEMFS_ASM)
	$(OBJDUMP) -t $(KERNELMEMFS_BIN) | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(KERNELMEMFS_SYM)

tags: $(OBJS) entryother.S _init
	etags *.S *.c

$(KERNEL_PLATFORM_X86)/vectors.S: $(KERNEL_PLATFORM_X86)/codegen/vectors.pl
	$(KERNEL_PLATFORM_X86)/codegen/vectors.pl > $(KERNEL_PLATFORM_X86)/vectors.S

ULIB = $(USER_LIB_DIR)/ulib.o $(USER_LIB_DIR)/usys.o $(USER_LIB_DIR)/printf.o $(USER_LIB_DIR)/umalloc.o $(USER_LIB_DIR)/structio.o $(USER_LIB_DIR)/modern.o

$(BUILD_USER_DIR)/_%: $(USER_BIN_DIR)/%.o $(ULIB) | $(BUILD_DIRS)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^
	$(OBJDUMP) -S $@ > $(BUILD_ARTIFACT_DIR)/$*.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(BUILD_ARTIFACT_DIR)/$*.sym

$(USER_BIN_DIR)/%.o: $(USER_BIN_DIR)/%.c
	$(CC) $(CFLAGS) -nostdinc -c -o $@ $<

$(USER_TEST_DIR)/%.o: $(USER_TEST_DIR)/%.c
	$(CC) $(CFLAGS) -nostdinc -c -o $@ $<

$(FS_STAGING_DIR)/bin/%: $(BUILD_USER_DIR)/% | $(BUILD_DIRS)
	mkdir -p $(dir $@)
	cp $< $@

$(STAGED_README): README | $(BUILD_DIRS)
	mkdir -p $(dir $@)
	cp $< $@

$(BUILD_USER_DIR)/_forktest: $(USER_TEST_DIR)/forktest.o $(ULIB) | $(BUILD_DIRS)
	# forktest has less library code linked in - needs to be small
	# in order to be able to max out the proc table.
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $(USER_TEST_DIR)/forktest.o $(USER_LIB_DIR)/ulib.o $(USER_LIB_DIR)/usys.o
	$(OBJDUMP) -S $@ > $(BUILD_ARTIFACT_DIR)/forktest.asm

$(MKFS_BIN): $(DEV_MKFS_DIR)/mkfs.c include/fs.h include/param.h | $(BUILD_DIRS)
	gcc -Werror -Wall -iquote $(INCLUDE_DIR) -o $@ $(DEV_MKFS_DIR)/mkfs.c

mkfs: $(MKFS_BIN)
	@echo "mkfs available at $(MKFS_BIN)"

# Prevent deletion of intermediate files, e.g. cat.o, after first build, so
# that disk image changes after first build are persistent until clean.  More
# details:
# http://www.gnu.org/software/make/manual/html_node/Chained-Rules.html
.PRECIOUS: %.o

UPROGS=\
	$(addprefix $(BUILD_USER_DIR)/_,$(USER_BINS))\
	$(BUILD_USER_DIR)/_forktest

$(FS_IMG): $(MKFS_BIN) $(STAGED_README) $(STAGED_UPROGS) | $(BUILD_DIRS)
	(cd $(FS_STAGING_DIR) && $(abspath $(MKFS_BIN)) $(abspath $@) $(STAGED_README_NAME) $(UPROG_STAGING_NAMES))

-include $(shell find $(SYSTEM_ROOT) user boot $(DEV_MKFS_DIR) $(BUILD_ARTIFACT_DIR) -name '*.d' 2>/dev/null)

clean: 
	rm -f *.tex *.dvi *.idx *.aux *.log *.ind *.ilg \
	bootasm.d bootmain.d entryother.d initcode.d \
	.gdbinit
	find $(SYSTEM_ROOT) user -name '*.o' -o -name '*.d' | xargs -r rm -f
	rm -f $(KERNEL_PLATFORM_X86)/vectors.S
	rm -rf $(BUILD_DIR)

# make a printout
FILES = $(shell grep -v '^\#' runoff.list)
PRINT = runoff.list runoff.spec README toc.hdr toc.ftr $(FILES)

xv6.pdf: $(PRINT)
	./runoff
	ls -l xv6.pdf

print: xv6.pdf

# run in emulators

bochs : fs.img xv6.img
	if [ ! -e .bochsrc ]; then ln -s dot-bochsrc .bochsrc; fi
	bochs -q

# try to generate a unique GDB port
GDBPORT = $(shell expr `id -u` % 5000 + 25000)
# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::$(GDBPORT)"; \
	else echo "-s -p $(GDBPORT)"; fi)
ifndef CPUS
CPUS := 2
endif
QEMUOPTS = -drive file=$(FS_IMG),index=1,media=disk,format=raw -drive file=$(XV6_IMG),index=0,media=disk,format=raw -smp $(CPUS) -m 512 $(QEMUEXTRA)

qemu: $(FS_IMG) $(XV6_IMG)
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

qemu-memfs: $(XV6_MEMFS_IMG)
	$(QEMU) -drive file=$(XV6_MEMFS_IMG),index=0,media=disk,format=raw -smp $(CPUS) -m 256

qemu-nox: $(FS_IMG) $(XV6_IMG)
	$(QEMU) -nographic $(QEMUOPTS)

.gdbinit: .gdbinit.tmpl | $(BUILD_DIRS)
	sed -e "s/localhost:1234/localhost:$(GDBPORT)/" \
	    -e "s|symbol-file kernel|symbol-file $(KERNEL_BIN)|" \
	    < $^ > $@

qemu-gdb: $(FS_IMG) $(XV6_IMG) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

qemu-nox-gdb: $(FS_IMG) $(XV6_IMG) .gdbinit
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUGDB)

# CUT HERE
# prepare dist for students
# after running make dist, probably want to
# rename it to rev0 or rev1 or so on and then
# check in that version.

EXTRA=\
	$(DEV_MKFS_DIR)/mkfs.c $(USER_LIB_DIR)/ulib.c include/user.h $(addprefix $(USER_BIN_DIR)/,$(addsuffix .c,$(USER_BINS)))\
	$(USER_TEST_DIR)/forktest.c $(USER_LIB_DIR)/printf.c $(USER_LIB_DIR)/umalloc.c\
	README dot-bochsrc $(KERNEL_PLATFORM_X86)/codegen/*.pl toc.* runoff runoff1 runoff.list\
	.gdbinit.tmpl $(DEV_DEBUG_DIR)/gdbutil\

dist:
	rm -rf dist
	mkdir dist
	for i in $(FILES); \
	do \
		grep -v PAGEBREAK $$i >dist/$$i; \
	done
	sed '/CUT HERE/,$$d' Makefile >dist/Makefile
	echo >dist/runoff.spec
	cp $(EXTRA) dist

dist-test:
	rm -rf dist
	make dist
	rm -rf dist-test
	mkdir dist-test
	cp dist/* dist-test
	cd dist-test; $(MAKE) print
	cd dist-test; $(MAKE) bochs || true
	cd dist-test; $(MAKE) qemu

# update this rule (change rev#) when it is time to
# make a new revision.
tar:
	rm -rf /tmp/xv6
	mkdir -p /tmp/xv6
	cp dist/* dist/.gdbinit.tmpl /tmp/xv6
	(cd /tmp; tar cf - xv6) | gzip >xv6-rev10.tar.gz  # the next one will be 10 (9/17)

.PHONY: dist-test dist
