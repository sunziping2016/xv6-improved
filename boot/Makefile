TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

OUTDIR = $(BUILDDIR)/boot

all: bootblock

bootblock: $(OUTDIR)/bootblock

$(OUTDIR)/bootblock: bootasm.S bootmain.c sign.pl | $(OUTDIR)
	$(CC) $(CFLAGS) -fno-pic -O -c bootmain.c -o $(OUTDIR)/bootmain.o
	$(CC) $(CFLAGS) -fno-pic -c bootasm.S -o $(OUTDIR)/bootasm.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 -o $(OUTDIR)/bootblock.o $(OUTDIR)/bootasm.o $(OUTDIR)/bootmain.o
	$(OBJDUMP) -S $(OUTDIR)/bootblock.o > $(OUTDIR)/bootblock.asm
	$(OBJCOPY) -S -O binary -j .text $(OUTDIR)/bootblock.o $@
	perl ./sign.pl $@

$(OUTDIR):
	mkdir -p $@

clean:
	rm -f $(OUTDIR)/*

.PHONY: all bootblock clean
