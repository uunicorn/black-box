
#include <stdio.h>
#include <unistd.h>

#include "ftdi-pic.h"

int
main()
{
    unsigned int d;

    init();
    reset();

    cmd(PIC_CMD_READ_PM);
    d = ind();
    printf("%04x?\n", d);
    sleep(5);

    cmd(PIC_CMD_ERASE_PM);
    sleep(1);
    
    cmd(PIC_CMD_READ_PM);
    d = ind();
    printf("%04x\n", d);

    return 0;
}
