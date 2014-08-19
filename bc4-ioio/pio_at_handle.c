
#include <message.h>
#include <pio.h>

#include "pio_at.h"
#include "pio.h"

void 
pio_handle_dir_get(Task task)
{
    pio_printf("\r\n+DIR=%u\r\n", PioGetDir() & PIO_USER);
    pio_printf("\r\nOK\r\n");
}

void 
pio_handle_dir_set(Task task, const struct pio_handle_dir_set *args)
{
    PioSetDir(PIO_USER, args->bits);
    pio_printf("\r\nOK\r\n");
}

void 
pio_handle_pin_get(Task task)
{
    pio_printf("\r\n+PIN=%u\r\n", PioGet() & PIO_USER);
    pio_printf("\r\nOK\r\n");
}

void 
pio_handle_pin_set(Task task, const struct pio_handle_pin_set *args)
{
    PioSet(PIO_USER, args->bits);
    pio_printf("\r\nOK\r\n");
}

void 
pio_handle_debounce_set(Task task, const struct pio_handle_debounce_set *args)
{
    PioDebounce((args->bits & PIO_USER) | PIO_BUTTONS, 2, 10); /* 2 times, 10ms */
    pio_printf("\r\nOK\r\n");
}

void 
pio_at_handleUnrecognised(const uint8 *data, uint16 length, Task task)
{
    pio_printf("\r\nERROR\r\n");
}
