
#include <stdio.h>
#include <unistd.h>

#include "ftdi-pic.h"

int
main()
{
    int a;
    unsigned int d, v;

    init();
    reset();

    cmd(PIC_CMD_LOAD_PM);
    d = ind();
    fprintf(stderr, "%04x?\n", d);
    sleep(5);

    reset();
    for(a=0;a < 0x2000;a++) {
        if(read(0, &d, 2) != 2)
            break;

        do {
            cmd(PIC_CMD_READ_PM);
            v = ind();

            if(v == d) {
                break;
            }

            fputc('w', stderr);
            fflush(stderr);

            cmd(PIC_CMD_LOAD_PM);
            outd(d);
            
            cmd(PIC_CMD_START_PROG);
            usleep(3000);
        } while(1);
        
        // next address
	cmd(PIC_CMD_INC_ADDR);

        fputc('.', stderr);
        fflush(stderr);
    }

    fputc('\n', stderr);

    return 0;
}
