TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

ULIB = ulib.o usys.o printf.o umalloc.o

all: ulib.a

install:

ulib.a: $(ULIB)
	$(AR) vr ulib.a $^
	$(RANLIB) ulib.a

clean:
	rm -f *.o ulib.a

.PHONY: all clean