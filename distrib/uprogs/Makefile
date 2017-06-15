TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

OUTDIR = $(BUILDDIR)/distrib/uprogs

UPROGS = cat echo forktest grep init kill ln ls \
		mkdir rm sh stressfs usertests wc zombie pwd reader


ULIB = $(OUTDIR)/../ulib/ulib.a
_UPROGS = $(addprefix $(OUTDIR)/, $(UPROGS))

all: $(_UPROGS)

$(ULIB): FORCE
	$(MAKE) -C ../ulib all

$(OUTDIR)/%.o: %.c | $(OUTDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

$(OUTDIR)/%: $(OUTDIR)/%.o $(ULIB) | $(OUTDIR)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

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
