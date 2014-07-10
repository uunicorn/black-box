
#include "pic16f628a.h"


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#define RA5_HOLE_BIT (1 << 5)
#define clrwdt __asm clrwdt __endasm

#ifndef KHZ
#define KHZ     4000
#endif

#define BAUD    9600
#define BAUD_HI 1

#if (BAUD_HI == 1)
#define BAUD_FACTOR  (16L*BAUD)
#else
#define BAUD_FACTOR  (64L*BAUD)
#endif

#define SPBRG_VALUE  (unsigned char)(((KHZ*1000L)-BAUD_FACTOR)/BAUD_FACTOR)

uint8_t segs[4];
uint8_t buf[4];

static void 
set_segment(uint8_t b)
{
    uint8_t v, m;
    uint8_t flush = 0;

    if(b & 0x80) {
        flush = 1;
        b &= 0x7f;
    }

    m = 0xf;
    v = b & m;

    if(b & 0x40) {
        v <<= 4;
        m <<= 4;
        b &= 0x3f;
    }

    b >>= 4;

    buf[b] = (buf[b] & ~m) | v;
    
    if(flush)
        for(b=0;b<4;b++)
            segs[b] = buf[b];
}

static void 
usart_rx()
{
    uint8_t b = RCREG;

    set_segment(b);

    TXREG = b;
    TXEN = 1;
}

static void isr() __interrupt 0
{
    if(RCIF) {
        usart_rx();
    }
}

void delay(uint16_t t)
{
    while(t) {
        t--;
    }
}

void main()
{
    uint8_t i;
    uint8_t bits[4];

    clrwdt;

    PORTA = 0;
    TRISA = 0x20; // RA5 - input only

    PORTB = 0x4;  // TX
    TRISB = 0x12; // RX + PGM

    SPBRG = SPBRG_VALUE;        // Baud Rate register, calculated by macro
    SYNC = 0;                        // Disable Synchronous/Enable Asynchronous
    BRGH = BAUD_HI;

    RX9 = 0;
    RCIF = 0;
    RCIE = 1;
    SPEN = 1;
    CREN = 1;

    PEIE=1;

    for(i=0;i<4;i++) {
        segs[i] = buf[i] = 0;
    }

    bits[0] = 1 << 0;
    bits[1] = 1 << 3;
    bits[2] = 1 << 6;
    bits[3] = 1 << 7;

    while(1) {
        GIE = 0;
        for(i=0;i<4;i++) {
            uint8_t s = segs[i];

            PORTA = s;
            RB5 = 0;
            PORTB |= s & RA5_HOLE_BIT;

            PORTB |= bits[i];
            delay(0x100);
            PORTB &= ~bits[i];
        }
        GIE = 1;
        clrwdt;
    }
}
