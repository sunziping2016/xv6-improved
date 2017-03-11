TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

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
	sleeplock.o\
	spinlock.o\
	string.o\
	swtch.o\
	syscall.o\
	sysfile.o\
	sysproc.o\
	timer.o\
	trap.o\
	uart.o\
	trapasm.o\
	vectors.o\
	vm.o\

all: kernel

entryother: entryother.S
	$(CC) $(CFLAGS) -fno-pic -c entryother.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7000 -o entryother.out entryother.o
	$(OBJCOPY) -S -O binary -j .text entryother.out entryother
	$(OBJDUMP) -S entryother.out > entryother.asm

initcode: initcode.S
	$(CC) $(CFLAGS) -c initcode.S
	$(LD) $(LDFLAGS) -N -e start -Ttext 0 -o initcode.out initcode.o
	$(OBJCOPY) -S -O binary initcode.out initcode
	$(OBJDUMP) -S initcode.o > initcode.asm

kernel: $(OBJS) entryother initcode kernel.ld
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel $(OBJS) -b binary initcode entryother
	$(OBJDUMP) -S kernel > kernel.asm
	$(OBJDUMP) -t kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > kernel.sym

vectors.S: vectors.pl
	perl vectors.pl > vectors.S

clean:
	rm -f *.o *.asm *.sym *.out entryother initcode vectors.S kernel

.PHONY: all clean