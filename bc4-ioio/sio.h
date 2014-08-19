
#define SIO_REENABLE_IND        3

typedef enum {
    SIO_INITIALIZING,
    SIO_IDLE,
    SIO_CONNECTING,
    SIO_CONNECTED
} sio_state_t;

void sio_init(Task task);
