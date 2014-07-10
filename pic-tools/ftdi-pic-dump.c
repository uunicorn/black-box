
#include <stdio.h>
#include <unistd.h>

#include "ftdi-pic.h"

int
main()
{
    int a;
    unsigned int d;

    init();
    reset();

    for(a=0; a < 0x200; a++) {
	cmd(PIC_CMD_READ_PM);
	d = ind();
	write(1, &d, 2);
	
	cmd(PIC_CMD_INC_ADDR);
    }

    return 0;
}
