/*
 * global.cc
 *
 * Copyright 2018 Robson Araújo Lima <robson@robson-lenovo>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

extern "C"
{
#include "ieee11073.h"
#include "manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util/log.h"
}

#include "m_plugin_castalia.h"
#include "m_global.h"
#include "MyPacket_m.h"

/**
 * (External var) Initial value for plugin id
 */
ContextId m_CONTEXT_ID = {2, 0};

/**
 *  (External var) Variable used by the stack
 */
unsigned long long m_port = 0; //not used for Castalia

static std::map<long, int> first_association;

static bool isManagerInitiatedModeActive[6] = {false};
static bool isNumberOfReceivedMeasurementsToSendStop[6] = {false};
static double numberOfReceivedMeasurementsToSendStop[6] = {0};

void setNumberOfReceivedMeasurementsToSendStop(double value, unsigned int nodeId){
    numberOfReceivedMeasurementsToSendStop[nodeId] = value;
}

void setIsNumberOfReceivedMeasurementsToSendStop(bool value, unsigned int nodeId){
    isNumberOfReceivedMeasurementsToSendStop[nodeId] = value;
}

bool getIsNumberOfReceivedMeasurementsToSendStop(unsigned int nodeId){
    return isNumberOfReceivedMeasurementsToSendStop[nodeId];
}

void setIsManagerInitiatedModeActive(bool value, unsigned int nodeId){
    isManagerInitiatedModeActive[nodeId] = value;
}

bool getIsManagerInitiatedModeActive(unsigned int nodeId){
    return isManagerInitiatedModeActive[nodeId];
}

/**
 * Callback function that is called whenever a device
 * is unvailable.
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void m_device_unavailable(Context *ctx)
{
    //fprintf(stderr, " main: Disasociated\n");
    DEBUG(" main: Disasociated\n");
}

/**
 * Callback function that is called whenever a new data
 * has been received.
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void new_data_received(Context *ctx, DataList *list)
{
    unsigned int nodeId = (ctx->id.plugin) / 2;
    //fprintf(stderr, "Medical Device Data Updated:\n");
    DEBUG("Medical Device Data Updated:\n");

    char *data = json_encode_data_list(list);

    if (data != NULL)
    {
        //fprintf(stderr, "%s", data);
        DEBUG("%s", data);
        //fprintf(stderr, "\n");
        DEBUG("\n");

    }

    fflush(stderr);
    free(data);

    // uncomment for manager-initiated disassociation testing
    // manager_request_association_release(m_CONTEXT_ID);
    
    //uncomment for manager-initiated stop message testing
    if (getIsNumberOfReceivedMeasurementsToSendStop(nodeId))
    {
        numberOfReceivedMeasurementsToSendStop[nodeId] = numberOfReceivedMeasurementsToSendStop[nodeId] -1;
        if(numberOfReceivedMeasurementsToSendStop[nodeId] <= 0.0)
        {
            DataReqMode mode = (DATA_REQ_START_STOP & 0x0000)
                           | DATA_REQ_SUPP_SCOPE_CLASS | DATA_REQ_SUPP_MODE_TIME_NO_LIMIT;
            manager_setDataReqMode(mode, nodeId);
            //cancel all the pending requests
            service_init(ctx);
            device_reqdata(ctx);
        }
    }

}

void new_data_received_from_manager_initiated_mode(Context *ctx, Request *r, DATA_apdu *response_apdu)
{
    //DataList *list = manager_get_mds_attributes(m_CONTEXT_ID);
    DataList *list = manager_get_mds_attributes(ctx->id);
    char *data = json_encode_data_list(list);

    //fprintf(stderr, "Medical Device Data Updated:\n");
    DEBUG("Medical Device Data Updated:\n");

    if (data != NULL)
    {
        //fprintf(stderr, "%s", data);
        DEBUG("%s", data);
        //fprintf(stderr, "\n");
        DEBUG("\n");

        fflush(stderr);
    }

    data_list_del(list);
    free(data);
}

/**
 * Callback function that is called whenever a new device
 * has been available
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void device_associated(Context *ctx, DataList *list)
{
    unsigned int nodeId = (ctx->id.plugin) / 2;

    //fprintf(stderr, " Medical Device System Associated:\n");
    DEBUG(" Medical Device System Associated:\n");

    char *data = json_encode_data_list(list);

    if (data != NULL)
    {
        //fprintf(stderr, "%s", data);
        DEBUG("%s", data);
        //fprintf(stderr, "\n");
        DEBUG("\n");

        fflush(stderr);
        free(data);
    }
    /*MDS attibutes is just needed in the first
     * association*/
    if (first_association[ctx->id.plugin] == 0)
    {
        device_reqmdsattr(ctx);
    }
    //Manager request measurements - Manager initiated mode
    if(getIsManagerInitiatedModeActive(nodeId))
    { 
        device_reqdata(ctx);
    }
}

/**
 * Callback function that is called whenever a new device
 * is connected to the Manager.
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void print_device_attributes(Context *ctx, Request *r, DATA_apdu *response_apdu)
{
    unsigned int nodeId = (ctx->id.plugin) / 2;
    //DataList *list = manager_get_mds_attributes(m_CONTEXT_ID);
    DataList *list = manager_get_mds_attributes(ctx->id);
    char *data = json_encode_data_list(list);

    //fprintf(stderr, "Print device attributes:\n");
    DEBUG("Print device attributes:\n");

    if (data != NULL)
    {
        //fprintf(stderr, "%s", data);
        DEBUG("%s", data);
        //fprintf(stderr, "\n");
        DEBUG("\n");

        fflush(stderr);
    }

    data_list_del(list);
    free(data);
    /*OK, attributes received, we do not need retrieve
    them anymore */
    first_association[ctx->id.plugin]++;

    // //Manager request measurements - Manager initiated mode
    // if(getIsManagerInitiatedModeActive(nodeId))
    //     device_reqdata(ctx);
}

/**
 * Request all MDS attributes
 *
 */
void device_reqmdsattr(Context *ctx)
{
    //fprintf(stderr, "device_reqmdsattr\n");
    DEBUG("device_reqmdsattr\n");
    manager_request_get_all_mds_attributes(ctx->id, print_device_attributes);
}

/**
 * Request agent data
 *
 */
void device_reqdata(Context *ctx)
{
    //fprintf(stderr, "device_reqdata\n");
    DEBUG("device_reqdata\n");
    manager_request_measurement_data_transmission(ctx->id, new_data_received_from_manager_initiated_mode);
}

/**
 * Waits 0 milliseconds for timeout.
 *
 * @param ctx current context.
 * @return fake timeout id
 */
int m_timer_count_timeout(Context *ctx)
{
    unsigned int nodeId = (ctx->id.plugin) / 2;
    m_SETTIMER[nodeId] = 1;
    return 1;
}

/**
 * Fake implementation of the reset timeout function.
 * @param ctx current context.
 */
void m_timer_reset_timeout(Context *ctx)
{
}

/**
 * Configure application to use castalia plugin
 */
void m_castalia_mode(unsigned int nodeId)
{
    m_CONTEXT_ID.plugin = 2;//no effect in Castalia application
    m_CONTEXT_ID.connid = m_port;//no effect in Castalia application
    m_plugin_network_castalia_manager_setup(&m_comm_plugin[nodeId], m_port);
    //Clean every initialization
    first_association.clear();
}
