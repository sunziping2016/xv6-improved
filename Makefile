TOP_SRCDIR = .
include $(TOP_SRCDIR)/Makefile.common

all: build/xv6.img build/fs.img

build/xv6.img: build/boot/bootblock build/kernel/kernel
	mkdir -p build
	dd if=/dev/zero of=build/xv6.img count=10000
	dd if=build/boot/bootblock of=build/xv6.img conv=notrunc
	dd if=build/kernel/kernel of=build/xv6.img seek=1 conv=notrunc

build/boot/bootblock: FORCE
	$(MAKE) -C boot bootblock

build/kernel/kernel: FORCE
	$(MAKE) -C kernel kernel

build/tools/mkfs: FORCE
	$(MAKE) -C tools mkfs

build/fs.img: build/fs/README build/tools/mkfs
	build/tools/mkfs build/fs.img build/fs

build/fs/README: FORCE
	mkdir -p build/fs
	$(MAKE) -C distrib install
	cp -u demo.basic build/fs/basic
	cp -u ORIG-README $@
	for i in $$(find build/fs -type f); do if [ $$i -nt $@ ]; then touch -r $$i $@; fi; done

clean:
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C distrib clean
	$(MAKE) -C tools clean
	rm -rf build/*

QEMUOPTS = -drive file=build/fs.img,index=1,media=disk,format=raw -drive file=build/xv6.img,index=0,media=disk,format=raw -smp $(CPUS) -m $(RAM) -soundhw all $(QEMUEXTRA)
QEMUGDB = -gdb tcp::$(GDBPORT)

qemu: build/fs.img build/xv6.img
	$(QEMU) -serial mon:stdio $(QEMUOPTS)

qemu-nox: build/fs.img build/xv6.img
	$(QEMU) -nographic $(QEMUOPTS)

qemu-gdb: build/fs.img build/xv6.img
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -serial mon:stdio $(QEMUOPTS) -S $(QEMUGDB)

qemu-nox-gdb: build/fs.img build/xv6.img
	@echo "*** Now run 'gdb'." 1>&2
	$(QEMU) -nographic $(QEMUOPTS) -S $(QEMUGDB)

.PHONY: all clean qemu qemu-nox qemu-gdb qemu-nox-gdb

FORCE:
