TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

OUTDIR = $(BUILDDIR)/tools

all: $(OUTDIR)/mkfs

mkfs: $(OUTDIR)/mkfs

$(OUTDIR)/mkfs: mkfs.c | $(OUTDIR)
	$(CC) -Wextra -Wall -O2 -o $@ $^ -iquote ../include

clean:
	rm -f $(OUTDIR)/*

$(OUTDIR):
	mkdir -p $(OUTDIR)

.PHONY: all mkfs clean
