TOP_SRCDIR = ../..
include $(TOP_SRCDIR)/Makefile.common

UPROGS = cat echo forktest grep init kill ln ls \
		mkdir rm sh stressfs usertests wc zombie

ULIB = ../ulib/ulib.a

all: $(UPROGS)

$(ULIB):
	$(MAKE) -C ../ulib ulib.a

# Cancel the .c -> output implicit rule
%: %.c

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%: %.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $@ $^

install: $(UPROGS)
	cp $(UPROGS) $(TOP_SRCDIR)/build/fs

clean:
	rm -f *.o *.asm *.sym $(UPROGS)

.PHONY: all install clean