
#include <message.h>
#include <source.h>
#include <vm.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <connection.h>
#include <pio.h>

#include "app.h"
#include "sio.h"

static Task app_task;
static TaskData sio_task;
static sio_state_t sio_state;
static uint32 sio_service_handle;

static void set_state(Task task, sio_state_t new_state)
{
    app_state_t *msg = PanicUnlessNew(app_state_t);
    *msg = sio_state = new_state;
    MessageSend(app_task, APP_SIO_STATE_CHANGED_IND, msg);
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
      0x1c, /* uuid128 af756630-1cdb-4324-a922-91609c8cd23b */
      0xaf, 0x75, 0x66, 0x30, 0x1c, 0xdb, 0x43, 0x24, 0xa9, 0x22, 0x91, 0x60, 0x9c, 0x8c, 0xd2, 0x3b, 
      0x09, /* uint16 0x0100 */
        0x01,
        0x00,
  0x09, /* ServiceName(0x0100) = "SIO" */
    0x01,
    0x00,
  0x25, /* String length 5 */
  0x03,
    'S','I','O'
};

static void sio_handle(Task task, MessageId id, Message message)
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

            sio_service_handle = msg->service_handle;

            set_state(task, SIO_IDLE);
            break;
        }

        /* ------------------RFCOMM events------------------- */
        case CL_RFCOMM_CONNECT_IND:
        {
            CL_RFCOMM_CONNECT_IND_T *msg = (CL_RFCOMM_CONNECT_IND_T*)message;

            if(sio_state == SIO_IDLE) {
                ConnectionRfcommConnectResponse(task,
                    TRUE,
                    &msg->bd_addr, 
                    msg->server_channel, 
                    0);
                set_state(task, SIO_CONNECTING);
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

            if(sio_state != SIO_CONNECTING)
                break;

            if(msg->status != rfcomm_connect_success) {
                set_state(task, SIO_IDLE);
                break;
            }

            set_state(task, SIO_CONNECTED);
            StreamConnect(StreamSourceFromSink(msg->sink), StreamUartSink());
            StreamConnect(StreamSourceFromSink(StreamUartSink()), msg->sink);
            break;
        }
        case CL_RFCOMM_DISCONNECT_IND:
        {
            CL_RFCOMM_DISCONNECT_IND_T *msg = (CL_RFCOMM_DISCONNECT_IND_T*)message;
            
            if(sio_state != SIO_CONNECTED)
                break;

            msg = msg;
            MessageSendLater(task, SIO_REENABLE_IND, 0, D_SEC(1));
            break;
        }
        case APP_REENABLE_IND:
        {
            set_state(task, SIO_IDLE);
            break;
        }
    }
}

void sio_init(Task task)
{
    app_task = task;
    sio_task.handler = sio_handle;
    sio_state = SIO_INITIALIZING;
    StreamUartConfigure(VM_UART_RATE_9K6, VM_UART_STOP_ONE, VM_UART_PARITY_NONE);
    ConnectionRfcommAllocateChannel(&sio_task);
}
