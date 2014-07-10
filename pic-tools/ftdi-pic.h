
#define PIC_CMD_LOAD_PM     2
#define PIC_CMD_READ_PM     4
#define PIC_CMD_INC_ADDR    6
#define PIC_CMD_START_PROG  8
#define PIC_CMD_ERASE_PM    9

int init();
void delay(unsigned long t);
void reset();
unsigned int ind();
void outd(unsigned int d);
void cmd(unsigned char d);
