/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/**
 * \file plugin_CASTALIA_agent.c
 * \brief CASTALIA plugin source.
 *
 * Copyright (C) 2011 Signove Tecnologia Corporation.
 * All rights reserved.
 * Contact: Signove Tecnologia Corporation (contact@signove.com)
 *
 * $LICENSE_TEXT:BEGIN$
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and appearing
 * in the file LICENSE included in the packaging of this file; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 * $LICENSE_TEXT:END$
 *
 *
 *
 * \author Elvis Pfutzenreuter
 * \author Adrian Guedes
 * \author Fabricio Silva Epaminondas
 * \date Jun 28, 2011
 */

/**
 * @addtogroup AgentCASTALIAPlugin
 * @{
 */
extern "C" {
#include "util/strbuff.h"
#include "communication/communication.h"
#include "communication/parser/decoder_ASN1.h"
#include "communication/parser/struct_cleaner.h"
#include "util/log.h"
#include "util/ioutil.h"
#include "asn1/phd_types.h"
#include "dim/nomenclature.h"
}

#include "MyPacket_m.h"
#include "plugin_castalia.h"
#include "global.h"
#include <netinet/in.h>
#include <map>

// #define TEST_FRAGMENTATION 1


unsigned int plugin_id = 1;

static const int CASTALIA_ERROR = NETWORK_ERROR;
static const int CASTALIA_ERROR_NONE = NETWORK_ERROR_NONE;
static const int BACKLOG = 1;

static int sk = -1;
static unsigned long long port = 0;
static intu8 *buffer = NULL;
static int buffer_size = 0;
static int buffer_retry = -1;

//vars used to collect outputs traces
static std::map<long,int> controlPackets;
static std::map<long,int> controlPacketsReceived;
static std::map<long,int> controlPacketsSent;
static std::map<long,int> measurementPackets;
static std::map<long,int> numberOfAssociations;


using namespace std;

/**
 * Initialize network layer.
 * Initialize network layer, in this case opens and initializes the CASTALIA socket.
 *
 * @return 1 if operation succeeds and 0 otherwise
 */
static int init_socket()
{
    sk = 3;//not used
    ContextId cid = {plugin_id, port};
    communication_transport_connect_indication(cid, "castalia");
    return 1;
}

/**
 * Initialize network layer, in this case opens and initializes
 *  the file descriptors
 *
 * @return CASTALIA_ERROR_NONE if operation succeeds
 */
static int network_init(unsigned int plugin_label)
{
    plugin_id = plugin_label;

    if (init_socket())
    {
        return CASTALIA_ERROR_NONE;
    }

    return CASTALIA_ERROR;
}

/**
 * Checks if the packet has more than one message
 *
 * @param ctx current connection context.
 * @return CASTALIA_ERROR_NONE if data is available or CASTALIA_ERROR if error.
 */
static int network_castalia_wait_for_data(Context *ctx)
{
    DEBUG("network castalia: network_wait_for_data");

    if (sk < 0)
    {
        DEBUG("network castalia: network_wait_for_data error");
        return CASTALIA_ERROR;
    }

    if (buffer_retry != 0)
    {
        // there may be another APDU in buffer already
        if (buffer_retry == -1)
            buffer_retry = buffer_retry + 1;
    }
    else
    {
        buffer_retry = -1;
        return CASTALIA_ERROR;
    }

    return CASTALIA_ERROR_NONE;
}

/**
 * Identifies the message type
 * 
 * name: message_type
 * @param current buffer, size and Context
 * @return void
 * 
 */
static void message_type(intu8 * buffer, int size, Context* ctx)
{
    unsigned int nodeId = (ctx->id.plugin+1) / 2;
    intu8 * bufferTmp = (intu8 *) calloc(size, sizeof(intu8));
    intu8 * initial_address = bufferTmp;
    for (int i = 0; i < size; i++)
    {
        bufferTmp[i] = buffer[i];
    }

    intu16 choice;
    choice = ntohs(*((uint16_t *) bufferTmp));
    bufferTmp += 8;

    switch (choice)
    {
    case AARQ_CHOSEN:
        st_msg[nodeId].msgType.push("Association Request");
        controlPackets[nodeId]++;
        numberOfAssociations[nodeId]++;
        break;
    case AARE_CHOSEN:
        st_msg[nodeId].msgType.push("Association Response");
        controlPackets[nodeId]++;
        break;
    case RLRQ_CHOSEN:
        st_msg[nodeId].msgType.push("Association Release Request");
        controlPackets[nodeId]++;
        break;
    case RLRE_CHOSEN:
        st_msg[nodeId].msgType.push("Association Release Response");
        controlPackets[nodeId]++;
        break;
    case ABRT_CHOSEN:
        st_msg[nodeId].msgType.push("Association Abort");
        controlPackets[nodeId]++;
        break;
    case PRST_CHOSEN:
    {
        choice = ntohs(*((uint16_t *) bufferTmp));

        switch (choice)
        {
        case ROIV_CMIP_EVENT_REPORT_CHOSEN:
        {
            /*uncomment if you want event type*/
            //bufferTmp += 10;
            //choice = ntohs(*((uint16_t *) bufferTmp));
            //int dec;
            //std::stringstream streamc;
            //streamc << choice;
            //streamc >> std::dec >> dec;
            //if (dec == MDC_NOTI_CONFIG){
            //st_msg[nodeId].msgType.push("Configuration with no confirmation");
            //}else{
            //st_msg[nodeId].msgType.push("Measurement with no confirmation");
            //}
            st_msg[nodeId].msgType.push("Measurement with no confirmation");
            measurementPackets[nodeId]++;
            break;
        }
        case ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN:
        {
            /*uncomment if you want event type*/
            //bufferTmp += 10;
            //choice = ntohs(*((uint16_t *) bufferTmp));
            //int dec;
            //std::stringstream streamc;
            //streamc << choice;
            //streamc >> std::dec >> dec;
            //if (dec == MDC_NOTI_CONFIG){
            //st_msg[nodeId].msgType.push("Configuration with confirmation");
            //}else{
            //st_msg[nodeId].msgType.push("Measurement with confirmation");
            //}
            st_msg[nodeId].msgType.push("Measurement with confirmation");
            measurementPackets[nodeId]++;
            break;
        }
        case ROIV_CMIP_GET_CHOSEN:
            st_msg[nodeId].msgType.push("GET configuration with confirmation");
            controlPackets[nodeId]++;
            break;
        case ROIV_CMIP_SET_CHOSEN:
            st_msg[nodeId].msgType.push("SET configuration with no confirmation");
            controlPackets[nodeId]++;
            break;
        case ROIV_CMIP_CONFIRMED_SET_CHOSEN:
            st_msg[nodeId].msgType.push("SET configuration with confirmation");
            controlPackets[nodeId]++;
            break;
        case ROIV_CMIP_ACTION_CHOSEN:
            st_msg[nodeId].msgType.push("ACTION with no confirmation");
            controlPackets[nodeId]++;
            break;
        case ROIV_CMIP_CONFIRMED_ACTION_CHOSEN:
            st_msg[nodeId].msgType.push("ACTION with confirmation");
            controlPackets[nodeId]++;
            break;
        case RORS_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN:
        {
            /*uncomment if you want event type*/
            //bufferTmp += 10;
            //choice = ntohs(*((uint16_t *) bufferTmp));
            //int dec;
            //std::stringstream streamc;
            //streamc << choice;
            //streamc >> std::dec >> dec;
            //if (dec == MDC_NOTI_CONFIG){
            //st_msg[nodeId].msgType.push("response of a configuration with confirmation");
            //}else{
            //st_msg[nodeId].msgType.push("response of a measurement with confirmation");
            //}
            st_msg[nodeId].msgType.push("response of a measurement with confirmation");
            controlPackets[nodeId]++;
            break;
        }
        case RORS_CMIP_GET_CHOSEN:
            st_msg[nodeId].msgType.push("response of a GET configuration");
            controlPackets[nodeId]++;
            break;
        case RORS_CMIP_CONFIRMED_SET_CHOSEN:
            st_msg[nodeId].msgType.push("response for a SET configuration with confirmation");
            controlPackets[nodeId]++;
            break;
        case RORS_CMIP_CONFIRMED_ACTION_CHOSEN:
            st_msg[nodeId].msgType.push("response of a ACTION with confirmation");
            controlPackets[nodeId]++;
            break;
        case ROER_CHOSEN:
            st_msg[nodeId].msgType.push("Remote Invoke Error");
            controlPackets[nodeId]++;
            break;
        case RORJ_CHOSEN:
            st_msg[nodeId].msgType.push("Remote Invoke Reject");
            controlPackets[nodeId]++;
            break;
        default:
            st_msg[nodeId].msgType.push("unknown data apdu choice");
            break;
        }
        break;
    }
    default:
        st_msg[nodeId].msgType.push("unknown data apdu choice");
        break;
    }

    /*
     * we are receiving a packet from manager
     * if st_msg[nodeId].tam_buff is 0, otherwise
     * we are sending
     * */
    if (st_msg[nodeId].tam_buff == 0)
    {
        controlPacketsReceived[nodeId] = controlPackets[nodeId] - controlPacketsSent[nodeId];
    }
    else
    {
        controlPacketsSent[nodeId] = controlPackets[nodeId] - controlPacketsReceived[nodeId];
    }
    free(initial_address);
}

/**
 * Reads an APDU from st_msg
 * @param ctx
 * @return a byteStream with the read APDU or NULL if error.
 */
static ByteStreamReader *network_get_apdu_stream(Context *ctx)
{
    ContextId cid = {plugin_id, port};
    unsigned int nodeId = (ctx->id.plugin+1) / 2;
    if (sk < 0)
    {
        ERROR("network CASTALIA: network_get_apdu_stream cannot found a valid sokcet");
        communication_transport_disconnect_indication(cid, "castalia");
        return NULL;
    }

    if (buffer_retry)
    {
        // handling letover data in buffer
        buffer_retry = 0;
    }
    else
    {
        int i;
        intu8 localbuf[65535];

        /*Transfer date from st_msg to localbuf*/
        for (i = 0; i < st_msg[nodeId].tam_buff; i++)
        {
            localbuf[i] = st_msg[nodeId].buff_msgRep[i];
        }

        int bytes_read = st_msg[nodeId].tam_buff;
        st_msg[nodeId].buff_msgRep.clear();
        st_msg[nodeId].tam_buff = 0;

        if (bytes_read < 0)
        {
            free(buffer);
            buffer = 0;
            buffer_size = 0;
            communication_transport_disconnect_indication(cid, "castalia");
            DEBUG(" network:CASTALIA error");
            sk = -1;
            return NULL;
        }
        else if (bytes_read == 0)
        {
            free(buffer);
            buffer = 0;
            buffer_size = 0;
            communication_transport_disconnect_indication(cid, "castalia");
            DEBUG(" network:CASTALIA closed");
            sk = -1;
            return NULL;
        }

        buffer = (intu8*) realloc(buffer, buffer_size + bytes_read);
        memcpy(buffer + buffer_size, localbuf, bytes_read);
        buffer_size += bytes_read;
    }

    if (buffer_size < 4)
    {
        DEBUG(" network:CASTALIA incomplete APDU (received %d)", buffer_size);
        return NULL;
    }

    int apdu_size = (buffer[2] << 8 | buffer[3]) + 4;

    if (buffer_size < apdu_size)
    {
        DEBUG(" network:CASTALIA incomplete APDU (expected %d received %d",
              apdu_size, buffer_size);
        return NULL;
    }

    // Create bytestream
    ByteStreamReader *stream = byte_stream_reader_instance(buffer, apdu_size);

    if (stream == NULL)
    {
        DEBUG(" network:CASTALIA Error creating bytelib");
        free(buffer);
        buffer = NULL;
        buffer_size = 0;
        return NULL;
    }

    buffer = 0;
    buffer_size -= apdu_size;
    if (buffer_size > 0)
    {
        // leave next APDU in buffer
        buffer_retry = 1;
        buffer = (intu8*) malloc(buffer_size);
        memcpy(buffer, stream->buffer_cur + apdu_size, buffer_size);
    }

    message_type(stream->buffer, apdu_size, ctx);
    DEBUG(" network:CASTALIA APDU received ");
    ioutil_print_buffer(stream->buffer_cur, apdu_size);

    return stream;
}

/**
 * Sends an encoded apdu
 *
 * @param ctx
 * @param stream the apdu to be sent
 * @return CASTALIA_ERROR_NONE if data sent successfully and CASTALIA_ERROR otherwise
 */
static int network_send_apdu_stream(Context *ctx, ByteStreamWriter *stream)
{
    unsigned int written = 0;
    unsigned int nodeId = (ctx->id.plugin + 1)/2;
    while (written < stream->size)
    {
        int to_send = stream->size - written;
#ifdef TEST_FRAGMENTATION
        to_send = to_send > 50 ? 50 : to_send;
#endif
        int ret = stream->size;
        DEBUG(" network:CASTALIA sent %d bytes", to_send);

        if (ret <= 0)
        {
            DEBUG(" network:CASTALIA Error sending APDU.");
            return CASTALIA_ERROR;
        }

        written += ret;
    }

    if ((stream->size) > 0)
    {
        unsigned int i;
        //Copy content from stream->buffer to st_msg
        for (i = 0; i < stream->size; i++)
        {
            st_msg[nodeId].buff_msgSed.push_back(stream->buffer[i]);
        }

        /**
         * If packet has two messages, the total packet size
         * is the size of msg1 + msg2 + ...
         **/
        st_msg[nodeId].tam_buff += stream->size;

    }

    message_type(stream->buffer, stream->size, ctx);
    DEBUG(" network:CASTALIA APDU sent ");
    ioutil_print_buffer(stream->buffer, stream->size);

    return CASTALIA_ERROR_NONE;
}

/**
 * Network disconnect
 *
 * @param ctx
 * @return CASTALIA_ERROR_NONE
 */
static int network_disconnect(Context *ctx)
{
    DEBUG("taking the initiative of disconnection");
    free(buffer);
    buffer = 0;
    buffer_size = 0;
    return CASTALIA_ERROR_NONE;
}

/**
 * Finalizes network layer and deallocated data
 *
 * @return CASTALIA_ERROR_NONE if operation succeeds
 */
static int network_finalize()
{
    free(buffer);
    buffer = 0;
    buffer_size = 0;
    return CASTALIA_ERROR_NONE;
}

/**
 * Initiate a CommunicationPlugin struct to use CASTALIA connection.
 *
 * @param plugin CommunicationPlugin pointer
 * @param pport Port of the socket
 *
 * @return CASTALIA_ERROR if error
 */
int plugin_network_castalia_agent_setup(CommunicationPlugin *plugin, int pport)
{
    DEBUG("network:castalia Initializing agent socket");
    plugin->network_init = network_init;
    plugin->network_wait_for_data = network_castalia_wait_for_data;
    plugin->network_get_apdu_stream = network_get_apdu_stream;
    plugin->network_send_apdu_stream = network_send_apdu_stream;
    plugin->network_disconnect = network_disconnect;
    plugin->network_finalize = network_finalize;

    return CASTALIA_ERROR_NONE;
}

int getControlPacketsSent(int addr)
{
    return controlPacketsSent[addr];
}
int getControlPacketsReceived(int addr)
{
    return controlPacketsReceived[addr];
}
int getMeasurementPacketsTotal(int addr)
{
    return measurementPackets[addr];
}
int getNumberOfAssociationsTotal(int addr)
{
    return numberOfAssociations[addr];
}

void clearVarMap(void)
{
    static int control = 0;
    /**
     * Clean up the control var every 5 initialization.
     * */
    if ((control % 5) == 0)
    {
        controlPackets.clear();
        controlPacketsSent.clear();
        controlPacketsReceived.clear();

        measurementPackets.clear();
        numberOfAssociations.clear();
        control = 0;
    }
    control++;
}

/** @} */
