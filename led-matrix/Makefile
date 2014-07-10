
CFLAGS=-S -V -mpic14 -p16f628a -D__16f628a  
LDFLAGS=-m -s /usr/share/gputils/lkr/16f628a.lkr /usr/share/sdcc/lib/pic14/libsdcc.lib
OBJS=led.o pic16f628a.o

all: led.bin

.SUFFIXES:

clean:
	rm -f *.asm *.o *.lst *.map *.hex *.cod

%.bin: %.hex
	objcopy -Iihex -Obinary $< $@

%.asm: %.c
	sdcc $(CFLAGS) $<

%.o: %.asm
	gpasm -c $<

led.hex: $(OBJS)
	gplink -o led.hex $(LDFLAGS) $(OBJS)
