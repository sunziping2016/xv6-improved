TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

OUTDIR = $(BUILDDIR)/kernel

OBJS = \
	bio.o\
	console.o\
	entry.o\
	exec.o\
	file.o\
	fs.o\
	ide.o\
	ioapic.o\
	kalloc.o\
	kbd.o\
	lapic.o\
	log.o\
	main.o\
	mp.o\
	picirq.o\
	pipe.o\
	proc.o\
  proc_table.o\
  proc_generic.o\
  proc_init.o\
  proc_rw.o\
	sleeplock.o\
	spinlock.o\
	string.o\
	swtch.o\
	syscall.o\
	tui.o\
	sysfile.o\
	sysproc.o\
	timer.o\
	trap.o\
	uart.o\
	trapasm.o\
	vectors.o\
	vm.o\

_OBJS = $(addprefix $(OUTDIR)/,$(OBJS))

all: kernel

kernel: $(OUTDIR)/kernel

$(OUTDIR)/entryother: entryother.S | $(OUTDIR)
	$(CC) $(CFLAGS) -fno-pic -c entryother.S -o $(OUTDIR)/entryother.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o $(OUTDIR)/entryother.out $(OUTDIR)/entryother.o
	$(OBJCOPY) -S -O binary -j .text $(OUTDIR)/entryother.out $(OUTDIR)/entryother
	$(OBJDUMP) -S $(OUTDIR)/entryother.out > $(OUTDIR)/entryother.asm

$(OUTDIR)/initcode: initcode.S | $(OUTDIR)
	$(CC) $(CFLAGS) -c initcode.S -o $(OUTDIR)/initcode.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o $(OUTDIR)/initcode.out $(OUTDIR)/initcode.o
	$(OBJCOPY) -S -O binary $(OUTDIR)/initcode.out $(OUTDIR)/initcode
	$(OBJDUMP) -S $(OUTDIR)/initcode.o > $(OUTDIR)/initcode.asm

$(OUTDIR)/kernel: $(_OBJS) $(OUTDIR)/entryother $(OUTDIR)/initcode kernel.ld | $(OUTDIR)
	$(LD) $(LDFLAGS) -T kernel.ld -o $@ $(_OBJS) -b binary $(OUTDIR)/initcode $(OUTDIR)/entryother
	$(OBJDUMP) -S $@ > $(OUTDIR)/kernel.asm
	$(OBJDUMP) -t $@ | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(OUTDIR)/kernel.sym

$(OUTDIR)/vectors.o: vectors.pl | $(OUTDIR)
	perl vectors.pl > $(OUTDIR)/vectors.S
	$(CC) $(OUTDIR)/vectors.S $(CFLAGS) -c -o $@

$(OUTDIR)/%.o: %.c | $(OUTDIR)
	$(CC) $< $(CFLAGS) -fno-pic -c -o $@

$(OUTDIR)/%.o: %.S | $(OUTDIR)
	$(CC) $< $(CFLAGS) -fno-pic -c -o $@

$(OUTDIR):
	mkdir -p $@

clean:
	rm -f $(OUTDIR)/*

.PHONY: all kernel clean
