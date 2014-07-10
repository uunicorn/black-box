
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

    cmd(0x00);
    outd(0x3fff);

    for(a=0x2000; a < 0x2007; a++) {
        cmd(PIC_CMD_READ_PM);
        d = ind();
        printf("%04x: %04x\n", a, d);

        cmd(PIC_CMD_INC_ADDR);
    }
    cmd(PIC_CMD_READ_PM);
    d = ind();
    printf("%04x: %04x\n", a, d);
/*
    cmd(PIC_CMD_ERASE_PM);
    sleep(5);
*/
    
    getchar();

    cmd(PIC_CMD_LOAD_PM);
    outd(0x3FDC);

    cmd(PIC_CMD_START_PROG);
    sleep(1);
    
    cmd(PIC_CMD_READ_PM);
    d = ind();
    printf("%04x: %04x\n", a, d);

    return 0;
}
