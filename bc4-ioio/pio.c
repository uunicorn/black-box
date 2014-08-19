
#include <message.h>
#include <source.h>
#include <vm.h>
#include <panic.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <connection.h>
#include <pio.h>

#include "app.h"
#include "pio.h"
#include "pio_at.h"

static Task app_task;
static TaskData pio_task;
static pio_state_t pio_state;
static uint32 pio_service_handle;
static Sink pio_sink;

static void set_state(Task task, pio_state_t new_state)
{
    app_state_t *msg = PanicUnlessNew(app_state_t);
    *msg = pio_state = new_state;
    MessageSend(app_task, APP_PIO_STATE_CHANGED_IND, msg);
}

static const unsigned char service_record[] =
{
  0x09, /* ServiceClassIDList(0x0001) */
    0x00,
    0x01,
  0x35, /* DataElSeq 3 bytes */
  0x03,
    0x19, /* uuid SerialPort(0x1101) */
    0x11,
    0x01,
  0x09, /* ProtocolDescriptorList(0x0004) */
    0x00,
    0x04,
  0x35, /* DataElSeq 12 bytes */
  0x0c,
    0x35, /* DataElSeq 3 bytes */
    0x03,
      0x19, /* uuid L2CAP(0x0100) */
      0x01,
      0x00,
    0x35, /* DataElSeq 5 bytes */
    0x05,
      0x19, /* uuid RFCOMM(0x0003) */
      0x00,
      0x03,
      0x08, /* uint8 0x00 */
        0x00,
  0x09, /* LanguageBaseAttributeIDList(0x0006) */
    0x00,
    0x06,
  0x35, /* DataElSeq 9 bytes */
  0x09,
    0x09, /* uint16 0x656e */
      0x65,
      0x6e,
    0x09, /* uint16 0x006a */
      0x00,
      0x6a,
    0x09, /* uint16 0x0100 */
      0x01,
      0x00,
  0x09, /* BluetoothProfileDescriptorList(0x0009) */
    0x00,
    0x09,
  0x35, /* DataElSeq 8 bytes */
  0x16,
    0x35, /* DataElSeq 6 bytes */
    0x14,
      0x1c, /* uuid128  5560049f-3b64-45ce-9707-be9680c7b8db */
	0x55, 0x60, 0x04, 0x9f, 0x3b, 0x64, 0x45, 0xce, 0x97, 0x07, 0xbe, 0x96, 0x80, 0xc7, 0xb8, 0xdb, 
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09, /* ServiceName(0x0100) = "PIO" */
    0x01,
    0x00,
  0x25, /* String length 5 */
  0x03,
    'P','I','O'
};

void pio_printf(const char *fmt, ...)
{
    char *p, buf[80];
    va_list ap;
    int l;
    unsigned short rc;

    if(!pio_sink)
	return;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    p = (char*)SinkMap(pio_sink);
    if(!p)
        return;

    l = strlen(buf);
    rc = SinkClaim(pio_sink, l);

    if(rc == 0xffff)
        return;

    p += rc;

    memcpy(p, buf, l);

    if(!SinkFlush(pio_sink, l))
        return;

    return;
}

static void pio_handle(Task task, MessageId id, Message message)
{
    switch(id) {
        /*----------------- connection events ------------------*/
        case CL_RFCOMM_REGISTER_CFM:
        {
            CL_RFCOMM_REGISTER_CFM_T *msg = (CL_RFCOMM_REGISTER_CFM_T*)message;
            uint8 *sr;
            
            if(msg->status != success)
                Panic();

            sr = PanicUnlessMalloc(sizeof(service_record));
            memcpy(sr, service_record, sizeof(service_record));
            
            sr[24] = msg->server_channel;

            ConnectionRegisterServiceRecord(task, sizeof(service_record), sr);
            break;
        }
        case CL_SDP_REGISTER_CFM:
        {
            CL_SDP_REGISTER_CFM_T *msg = (CL_SDP_REGISTER_CFM_T*)message;

            if(msg->status != success)
                Panic();

            pio_service_handle = msg->service_handle;

            set_state(task, PIO_IDLE);
            break;
        }

        /* ------------------RFCOMM events------------------- */
        case CL_RFCOMM_CONNECT_IND:
        {
            CL_RFCOMM_CONNECT_IND_T *msg = (CL_RFCOMM_CONNECT_IND_T*)message;

            if(pio_state == PIO_IDLE) {
                ConnectionRfcommConnectResponse(task,
                    TRUE,
                    &msg->bd_addr, 
                    msg->server_channel, 
                    0);
                set_state(task, PIO_CONNECTING);
            }
	    else {
                ConnectionRfcommConnectResponse(task,
                    FALSE,
                    &msg->bd_addr, 
                    msg->server_channel, 
                    0);
	    }

            break;
        }
        case CL_RFCOMM_CONNECT_CFM:
        {
            CL_RFCOMM_CONNECT_CFM_T *msg = (CL_RFCOMM_CONNECT_CFM_T*)message;

            if(pio_state != PIO_CONNECTING)
                break;

            if(msg->status != rfcomm_connect_success) {
                set_state(task, PIO_IDLE);
                break;
            }

            set_state(task, PIO_CONNECTED);
	    pio_sink = msg->sink;
	    pio_printf("\r\n+HELLO\r\n");
            break;
        }
        case CL_RFCOMM_DISCONNECT_IND:
        {
            CL_RFCOMM_DISCONNECT_IND_T *msg = (CL_RFCOMM_DISCONNECT_IND_T*)message;
            
            if(pio_state != PIO_CONNECTED)
                break;

	    pio_sink = NULL;
            msg = msg;
            MessageSendLater(task, PIO_REENABLE_IND, 0, D_SEC(1));
            break;
        }
        case APP_REENABLE_IND:
        {
            set_state(task, PIO_IDLE);
            break;
        }

        /* ------------------System events------------------- */
        case MESSAGE_MORE_DATA:
	{
	    Source src = ((MessageMoreData*)message)->source;
	    pio_at_parseSource(src, task);
	    break;
	}
        /* ------------------PIO events------------------- */

        case MESSAGE_PIO_CHANGED:
        {
            static unsigned short prev_state = 0xffff;
            MessagePioChanged *msg = (MessagePioChanged *)message;
            unsigned short pressed = prev_state & ~msg->state;
            unsigned short released = ~prev_state & msg->state;
            unsigned short changed = prev_state ^ msg->state;

	    if(pressed & PIO_DISCOVERABLE_BUTTON) {
		MessageSend(app_task, APP_DISCOVERABLE_PRESSED, 0);
	    }

	    if(released & PIO_DISCOVERABLE_BUTTON) {
		MessageSend(app_task, APP_DISCOVERABLE_RELEASED, 0);
	    }

	    if(changed & PIO_USER) {
		pio_printf("\r\n+CHANGED=%u,%u\r\n", prev_state & PIO_USER, msg->state & PIO_USER);
	    }

            prev_state = msg->state;
            break;
        }

    }
}

void pio_init(Task task)
{
    app_task = task;
    pio_task.handler = pio_handle;
    pio_state = PIO_INITIALIZING;

    PioSetDir(PIO_OUTS, ~0);
    PioSet(PIO_OUTS, 0);

    PioSetDir(PIO_BUTTONS, 0);
    PioSet(PIO_BUTTONS, ~0);
    PioDebounce(PIO_BUTTONS, 2, 10); /* 2 times, 10ms */
    MessagePioTask(&pio_task);

    ConnectionRfcommAllocateChannel(&pio_task);
}
