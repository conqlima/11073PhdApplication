#ifndef _GLOBAL_H_
#define _GLOBAL_H_

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

typedef struct Tmsg
{
    queueOfMsgType msgType;
    streamOfByte buff_msgRep;//buff for REcPtion
    streamOfByte buff_msgSed;//buff for SEnDing
    int tam_buff = 0;
} Tmsg;

/**
 * Plugin used by agent to send network data
 */
extern ContextId CONTEXT_ID;

/**
 * PLugin definition
 */
extern CommunicationPlugin* comm_plugin;

/**
 * Struct to represent the messages exchanged
 * one struct for each node, the position 0 is invalid
 * */
extern Tmsg* st_msg;

extern int* SETTIMER;

extern int HUBNODE;

void timer_reset_timeout(Context *ctx);
int timer_count_timeout(Context *ctx);
void device_connected(Context *ctx, const char *addr);
void device_unavailable(Context *ctx);
void device_associated(Context *ctx);
void sigalrm(int dummy);
void castalia_mode(unsigned int nodeNumber);

#endif
