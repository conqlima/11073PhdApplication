#ifndef _M_GLOBAL_H_
#define _M_GLOBAL_H_

extern "C" {
#include "communication/context.h"
#include "api/api_definitions.h"
#include "communication/plugin/plugin.h"
#include "communication/service.h"
}

#include <vector>
#include <queue>

typedef std::queue<const char*> queueOfMsgType;
typedef std::vector<uint8_t> streamOfByte;

typedef struct m_Tmsg
{
    queueOfMsgType msgType;
    streamOfByte buff_msgRep;
    streamOfByte buff_msgSed;
    int tam_buff = 0;
} m_Tmsg;

/**
 * 
 */
extern ContextId m_CONTEXT_ID;

/**
 * PLugin definition
 */
extern CommunicationPlugin* m_comm_plugin;

/**
 * Variable used by the stack
 */
extern unsigned long long m_port;//Not used for Castalia

/**
 * Struct to represent the messages
 */
extern m_Tmsg* m_st_msg;

/**
 * Control de timeouts
 */
extern int* m_SETTIMER;

void m_timer_reset_timeout(Context *ctx);
int m_timer_count_timeout(Context *ctx);
void device_associated(Context *ctx, DataList *list);
void m_device_unavailable(Context *ctx);
void device_reqmdsattr();
void device_reqdata();
//void new_data_received_test(Context *ctx, Request *r, DATA_apdu *response_apdu);
void new_data_received(Context *ctx, DataList *list);
void print_device_attributes(Context *ctx, Request *r, DATA_apdu *response_apdu);
void m_castalia_mode(unsigned int my_plugin_number);

#endif
