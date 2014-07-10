

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>

#include <ftdi.h>
#include <stdio.h>

unsigned char data;

struct ftdi_context ftdic;

void 
ftdi_fatal(char *str)
{
    fprintf(stderr, "%s: %s\n", str, ftdi_get_error_string (&ftdic));
    exit(1);
}

static void
fini(int rc, void *arg)
{
    data = 0;

    if (ftdi_write_data(&ftdic, &data, 1) < 0)
        fprintf (stderr, "on_exit: write: %s\n",
             ftdi_get_error_string (&ftdic));

    ftdi_deinit(&ftdic);
}

int 
init()
{
    ftdi_init(&ftdic);

    ftdi_set_interface(&ftdic, INTERFACE_B);

    if(ftdi_usb_open(&ftdic, 0x0403, 0x6010) < 0)
        ftdi_fatal("Can't open device");

    ftdi_usb_reset(&ftdic);
    ftdi_usb_purge_buffers(&ftdic);

    if (ftdi_set_bitmode(&ftdic, 0x0d, BITMODE_BITBANG) < 0)
        ftdi_fatal("Can't enable bitbang");

    data = 0;

    on_exit(fini, NULL);

    return 0;
}

static void 
out() 
{
    if (ftdi_write_data(&ftdic, &data, 1) < 0)
        ftdi_fatal("Can't write");
}

void
reset()
{
    data &= ~8;
    out();

    usleep(10000);

    data |= 8;
    out();

    usleep(100000);
}

void
pgc(int on)
{
    if(on)
        data |= 4;
    else
        data &= ~4;

    out();
    //usleep(100);
}

void
pgd(int on)
{
    if(on)
        data |= 1;
    else
        data &= ~1;

    out();
}

int
pgdin()
{
    uint8_t b;

    if (ftdi_read_pins(&ftdic, &b) < 0) 
        ftdi_fatal("Can't read");

    return (b & 0x2) != 0;
}

void
delay(unsigned long t)
{
    struct timeval now, then = { 0, t };

    //usleep(t*1000);
    //return;
    gettimeofday(&now, NULL);
    timeradd(&now, &then, &then);
    do {
        gettimeofday(&now, NULL);
    } while(timercmp(&now, &then, <));
}


void
cmd(unsigned char d)
{
    int x;

    for(x=0; x < 6;x++, d >>= 1) {
        pgd((d & 1) != 0);
        pgc(1);
        pgc(0);
    }
    pgd(1);
    delay(1);
}

void
outd(unsigned int d)
{
    int x;
    
    d <<= 1;
    d &= 0x7ffe;

    for(x=0; x < 16; x++, d >>= 1) {
        pgd((d & 1) != 0);
        pgc(1);
        pgc(0);
        
    }
    pgd(1);
    delay(1);
}

unsigned int
ind()
{
    unsigned int x, rc;

    rc = 0;
    
    pgd(0);
    for(x=0; x < 16; x++) {
        pgc(1);
        pgc(0);
 
        rc >>= 1;

        if(pgdin( ))
            rc |= 0x8000;
    }
    
    if((rc & 0x8001) != 0) {
        fprintf(stderr, "assertion failed for start/stop bit: %04x\n", rc);
        exit(1);
    }
    
    delay(1);
    return rc >> 1;
}
