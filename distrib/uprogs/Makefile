TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

OUTDIR = $(BUILDDIR)/distrib/uprogs

UPROGS = cat echo forktest grep init kill ln ls \
		mkdir rm sh stressfs usertests wc zombie \
		mount pwd reader stdtests jerry timetest vi bi \
		semtest rwtest demo

ULIB = $(OUTDIR)/../ulib/ulib.a
LIBC = $(OUTDIR)/../libc/libc.a
START = _start.o
_UPROGS = $(addprefix $(OUTDIR)/, $(UPROGS))

all: $(_UPROGS)

$(ULIB): FORCE
	$(MAKE) -C ../ulib all

$(LIBC): FORCE
	$(MAKE) -C ../libc all

$(OUTDIR)/%.o: %.c | $(OUTDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUTDIR)/%: $(OUTDIR)/%.o $(START) $(LIBC) $(ULIB)  | $(OUTDIR)
	$(LD) $(LDFLAGS) -s -N -e _start -Ttext 0 -o $@ $^

install: $(_UPROGS)
	mkdir -p $(TOP_SRCDIR)/build/fs
	cp -u $(_UPROGS) $(TOP_SRCDIR)/build/fs

$(OUTDIR):
	mkdir -p $(OUTDIR)

clean:
	rm -f $(OUTDIR)/*

.PHONY: all install clean

.PRECIOUS: $(OUTDIR)/%.o

FORCE:
