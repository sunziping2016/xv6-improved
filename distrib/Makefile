TOP_SRCDIR = ..
include $(TOP_SRCDIR)/Makefile.common

all:
	$(MAKE) -C ulib all
	$(MAKE) -C uprogs all

install:
	$(MAKE) -C uprogs install

clean:
	$(MAKE) -C ulib clean
	$(MAKE) -C uprogs clean

.PHONY: all install clean