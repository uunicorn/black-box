
#include <message.h>
#include <source.h>
#include <vm.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <connection.h>
#include <spp.h>
#include <pio.h>

#include "app.h"
#include "sio.h"
#include "pio.h"
#include "pio_at.h"

static TaskData con_task;
static app_state_t app_state;
static char is_discoverable;

static void led_update(Task task)
{
    MessageCancelAll(task, APP_LED_IND);
    MessageSend(task, APP_LED_IND, 0);
}

static void set_state(Task task, app_state_t new_state, char inquire)
{
    app_state = new_state;
    is_discoverable = inquire;
    led_update(task);
}

static void conn_handle(Task task, MessageId id, Message message)
{
    switch(id) {
        /*----------------- connection events ------------------*/
        case CL_INIT_CFM:
        {
            CL_INIT_CFM_T *msg = (CL_INIT_CFM_T*)message;

            if (msg->status != success)
                Panic();
            
	    sio_init(task);
	    pio_init(task);
            ConnectionSmSetSdpSecurityIn(TRUE);
            ConnectionWriteScanEnable(hci_scan_enable_page);
            break;
        }

        case CL_SM_PIN_CODE_IND: 
            if(is_discoverable) {
                CL_SM_PIN_CODE_IND_T *msg = (CL_SM_PIN_CODE_IND_T*)message;
                ConnectionSmPinCodeResponse(&msg->bd_addr, 4, (uint8*)"0000");
            }
            break;
        case CL_SM_AUTHORISE_IND:
        {
            CL_SM_AUTHORISE_IND_T *msg = (CL_SM_AUTHORISE_IND_T*)message;
            ConnectionSmAuthoriseResponse(&msg->bd_addr, msg->protocol_id, msg->channel, msg->incoming, TRUE);
            break;
        }

        /* ------------------custom events------------------- */
	case APP_SIO_STATE_CHANGED_IND:
	{
	    sio_state_t *sio_state = (sio_state_t *)message;
	    switch(*sio_state) {
		case SIO_CONNECTED:
		    set_state(task, APP_CONNECTED, is_discoverable);
		    break;
		case SIO_IDLE:
		    set_state(task, APP_IDLE, is_discoverable);
		    break;
		default:
		    /* ignore */
		    ;
	    }
	    break;
	}
	case APP_DISCOVERABLE_PRESSED:
	{
            if(!is_discoverable) {
		set_state(task, app_state, TRUE);
		ConnectionWriteScanEnable(hci_scan_enable_inq_and_page);
            }

            MessageCancelAll(task, APP_INQUIRE_TIMEOUT_IND);
            MessageSendLater(task, APP_INQUIRE_TIMEOUT_IND, 0, D_MIN(1));
            MessageCancelAll(task, APP_DELETE_ALL_AUTH_IND);
            MessageSendLater(task, APP_DELETE_ALL_AUTH_IND, 0, D_SEC(2));
	    break;
	}
	case APP_DISCOVERABLE_RELEASED:
	{
	    MessageCancelAll(task, APP_DELETE_ALL_AUTH_IND);
	    break;
	}
	case APP_DELETE_ALL_AUTH_IND:
	{
            ConnectionSmDeleteAllAuthDevices(0);
            MessageCancelAll(task, APP_DELETE_ALL_AUTH_IND);
	    break;
	}
        case APP_INQUIRE_TIMEOUT_IND:
        {
            if(is_discoverable) {
                ConnectionWriteScanEnable(hci_scan_enable_page);
                set_state(task, app_state, FALSE);
                MessageCancelAll(task, APP_INQUIRE_TIMEOUT_IND);
            }
            break;
        }
        case APP_LED_IND:
        {
            uint16 led;

            switch(app_state) {
            case APP_CONNECTED:
                led = PIO_LED;
                break;
            case APP_IDLE:
                if(is_discoverable) {
                    led = PioGet() ^ PIO_LED;
                    MessageSendLater(task, APP_LED_IND, 0, D_SEC(1)/2);
                    break;
                }
                /* fall through */
            default:
                led = 0;
                break;
            }

            PioSet(PIO_LED, led);
            break;
        }
        case APP_REENABLE_IND:
        {
            set_state(task, APP_IDLE, is_discoverable);
            break;
        }
    }
}

int main(void)
{
    con_task.handler = conn_handle;
    app_state = APP_INITIALIZING;
    ConnectionInit(&con_task);

    MessageLoop();

    return 0;
}
