
#define PIO_REENABLE_IND        3

#define PIO_AUX_BUTTON          (1 << 6)
#define PIO_DISCOVERABLE_BUTTON (1 << 8)
#define PIO_BUTTONS (PIO_AUX_BUTTON|PIO_DISCOVERABLE_BUTTON)

#define PIO_LED                 (1 << 9) 
#define PIO_LED_AUX             (1 << 7) 
#define PIO_LEDS (PIO_LED|PIO_LED_AUX)

#define PIO_OUTS (PIO_LEDS)

#define PIO_USER (~(PIO_OUTS | PIO_BUTTONS))

typedef enum {
    PIO_INITIALIZING,
    PIO_IDLE,
    PIO_CONNECTING,
    PIO_CONNECTED
} pio_state_t;

void pio_init(Task task);
void pio_printf(const char *fmt, ...);
