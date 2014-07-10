
TOOLS=flash erase word dump
TOOLS_CFLAGS=-m32 -lftdi -Wall

all: tools

tools: $(TOOLS)

flash: ftdi-pic.h ftdi-pic.c ftdi-pic-flash.c
	cc ftdi-pic.c ftdi-pic-flash.c $(TOOLS_CFLAGS) -o $@

erase: ftdi-pic.h ftdi-pic.c ftdi-pic-erase.c
	cc ftdi-pic.c ftdi-pic-erase.c $(TOOLS_CFLAGS) -o $@

word: ftdi-pic.h ftdi-pic.c ftdi-pic-word.c
	cc ftdi-pic.c ftdi-pic-word.c $(TOOLS_CFLAGS) -o $@

dump: ftdi-pic.h ftdi-pic.c ftdi-pic-dump.c
	cc ftdi-pic.c ftdi-pic-dump.c $(TOOLS_CFLAGS) -o $@

