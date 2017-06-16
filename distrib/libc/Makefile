TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

OUTDIR = $(BUILDDIR)/distrib/libc

ULIB = string.o math.o stdlib.o stdio.o scanf.o ctype.o

_ULIB = $(addprefix $(OUTDIR)/, $(ULIB))

all: $(OUTDIR)/libc.a

install:

$(OUTDIR)/libc.a: $(_ULIB) | $(OUTDIR)
	$(AR) vr $@ $^
	$(RANLIB) $@

$(OUTDIR)/%.o: %.c | $(OUTDIR)
	$(CC) $< $(CFLAGS) -c -o $@

clean:
	rm -f $(OUTDIR)/*

$(OUTDIR):
	mkdir -p $(OUTDIR)

.PHONY: all install clean
