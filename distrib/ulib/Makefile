TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

OUTDIR = $(BUILDDIR)/distrib/ulib

ULIB = ulib.o usys.o printf.o

_ULIB = $(addprefix $(OUTDIR)/, $(ULIB))

all: $(OUTDIR)/ulib.a

install:

$(OUTDIR)/ulib.a: $(_ULIB) | $(OUTDIR)
	$(AR) vr $@ $^
	$(RANLIB) $@

$(OUTDIR)/%.o: %.c | $(OUTDIR)
	$(CC) $< $(CFLAGS) -c -o $@

$(OUTDIR)/%.o: %.S | $(OUTDIR)
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -f $(OUTDIR)/*

$(OUTDIR):
	mkdir -p $(OUTDIR)

.PHONY: all install clean
