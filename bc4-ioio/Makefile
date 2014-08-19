# Dont forget to export BLUELAB="c:/BlueLab/tools"


TRANSPORT=user
DEBUGTRANSPORT=[SPI|LPT1|No]
EXECUTION_MODE=vm
INPUTS=app.c sio.c pio.c pio_at.c pio_at_handle.c
OUTPUT=test
STACKSIZE=500
CFLAGS=-O99
LIBS= -lbdaddr -lconnection
HARDWARE=coyote
FLASHSIZE=8192
#65536


include $(BLUELAB)/Makefile.mode.vm
include $(BLUELAB)/Makefile.vm
include $(BLUELAB)/Makefile.rules
