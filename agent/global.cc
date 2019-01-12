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
extern "C" {
#include <ieee11073.h>
#include "agent.h"
#include "util/log.h"
}

#include "plugin_castalia.h"
#include "global.h"
#include "MyPacket_m.h"

//Initial value for plugin id
ContextId CONTEXT_ID = {1, 0};

/**
 * Waits 0 milliseconds for timeout.
 *
 * @param ctx current context.
 * @return fake timeout id
 */
int timer_count_timeout(Context *ctx)
{
    unsigned int nodeId = (ctx->id.plugin+1) / 2;
    SETTIMER[nodeId] = 1;
    return 1;
}

/**
 * Fake implementation of the reset timeout function.
 * @param ctx current context.
 */
void timer_reset_timeout(Context *ctx)
{
}

/**
 * Callback function that is called whenever a
 * Divece is associated with a Manager.
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void device_associated(Context *ctx)
{
    //fprintf(stderr, " main: Associated\n");
    DEBUG(" main: Associated\n");
}

/**
 * Callback function that is called whenever a device
 * is unvailable.
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void device_unavailable(Context *ctx)
{
    //fprintf(stderr, " main: Disasociated\n");
    DEBUG(" main: Disasociated\n");
}

/**
 * Callback function that is called whenever a
 * Divece is initiated (connected).
 *
 * @param ctx current context.
 * @param list the new list of elements.
 */
void device_connected(Context *ctx, const char *addr)
{
    //fprintf(stderr, "main: Connected\n");
    DEBUG("main: Connected\n");

    // ok, make it proceed with association
    // (agent has the initiative)
    agent_associate(ctx->id);
}

/**
 * Configure application to use castalia plugin
 */
void castalia_mode(unsigned int nodeNumber)
{
    int port = 0;
    CONTEXT_ID.plugin = 1;
    CONTEXT_ID.connid = port;
    plugin_network_castalia_agent_setup(&comm_plugin[nodeNumber], port);
}
