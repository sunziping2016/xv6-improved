TOP_SRCDIR = .
include $(TOP_SRCDIR)/Makefile.common

all: build/xv6.img build/fs.img

build/xv6.img: boot/bootblock kernel/kernel
	dd if=/dev/zero of=build/xv6.img count=10000
	dd if=boot/bootblock of=build/xv6.img conv=notrunc
	dd if=kernel/kernel of=build/xv6.img seek=1 conv=notrunc

boot/bootblock:
	$(MAKE) -C boot bootblock

kernel/kernel:
	$(MAKE) -C kernel kernel

tools/mkfs:
	$(MAKE) -C tools mkfs

build/fs.img: tools/mkfs
	mkdir -p build/fs
	cp ORIG-README build/fs/README
	$(MAKE) -C distrib install
	tools/mkfs build/fs.img build/fs

clean:
	$(MAKE) -C boot clean
	$(MAKE) -C kernel clean
	$(MAKE) -C tools clean
	$(MAKE) -C distrib clean
	rm -rf build/*

QEMUOPTS = -drive file=build/fs.img,index=1,media=disk,format=raw -drive file=build/xv6.img,index=0,media=disk,format=raw -smp $(CPUS) -m $(RAM) $(QEMUEXTRA)
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

.PHONY: all clean qemu qemu-nox qemu-gdb qemu-nox-gdb boot/bootblock kernel/kernel tools/mkfs build/fs.img