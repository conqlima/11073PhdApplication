/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/**
 * \file sample_agent_common.c
 * \brief Main application implementation.
 *
 * Copyright (C) 2012 Signove Tecnologia Corporation.
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
 * \author Elvis Pfutzenreuter
 * \date Apr 17, 2012
 */
extern "C" {
#include <ieee11073.h>
#include "specializations/pulse_oximeter.h"
#include "specializations/blood_pressure_monitor.h"
#include "specializations/weighing_scale.h"
#include "specializations/glucometer.h"
#include "specializations/thermometer.h"
#include "specializations/basic_ECG.h"
#include "agent.h"
#include <netinet/in.h>
}

#include "sample_agent_common.h"
#include <cstring>

intu8 AGENT_SYSTEM_ID_VALUE[] = { 0x11, 0x33, 0x55, 0x77, 0x99,
                                  0xbb, 0xdd, 0xff
                                };

/**
 * Generate data for oximeter event report
 */
void *oximeter_event_report_cb()
{
    time_t now;
    struct tm nowtm;
    struct oximeter_event_report_data* data =
        (oximeter_event_report_data*) malloc(sizeof(struct oximeter_event_report_data));

    time(&now);
    localtime_r(&now, &nowtm);

    data->beats = 60.5 + random() % 20;
    data->oximetry = 90.5 + random() % 10;
    data->century = nowtm.tm_year / 100 + 19;
    data->year = nowtm.tm_year % 100;
    data->month = nowtm.tm_mon + 1;
    data->day = nowtm.tm_mday;
    data->hour = nowtm.tm_hour;
    data->minute = nowtm.tm_min;
    data->second = nowtm.tm_sec;
    data->sec_fractions = 50;

    return data;
}

/**
 * Generate data for blood pressure event report
 */
void *blood_pressure_event_report_cb()
{
    time_t now;
    struct tm nowtm;
    struct blood_pressure_event_report_data* data =
        (blood_pressure_event_report_data*) malloc(sizeof(struct blood_pressure_event_report_data));

    time(&now);
    localtime_r(&now, &nowtm);

    data->systolic = 110 + random() % 30;
    data->diastolic = 70 + random() % 20;
    data->mean = 90 + random() % 10;
    data->pulse_rate = 60 + random() % 30;

    data->century = nowtm.tm_year / 100 + 19;
    data->year = nowtm.tm_year % 100;
    data->month = nowtm.tm_mon + 1;
    data->day = nowtm.tm_mday;
    data->hour = nowtm.tm_hour;
    data->minute = nowtm.tm_min;
    data->second = nowtm.tm_sec;
    data->sec_fractions = 50;

    return data;
}

/**
 * Generate data for weight scale event report
 */
void *weightscale_event_report_cb()
{
    time_t now;
    struct tm nowtm;
    struct weightscale_event_report_data* data =
        (weightscale_event_report_data*) malloc(sizeof(struct weightscale_event_report_data));

    time(&now);
    localtime_r(&now, &nowtm);

    data->weight = 70.2 + random() % 20;
    data->bmi = 20.3 + random() % 10;

    data->century = nowtm.tm_year / 100 + 19;
    data->year = nowtm.tm_year % 100;
    data->month = nowtm.tm_mon + 1;
    data->day = nowtm.tm_mday;
    data->hour = nowtm.tm_hour;
    data->minute = nowtm.tm_min;
    data->second = nowtm.tm_sec;
    data->sec_fractions = 50;

    return data;
}

/**
 * Generate data for Glucometer event report
 */
void *glucometer_event_report_cb()
{
    time_t now;
    struct tm nowtm;
    struct glucometer_event_report_data* data =
        (glucometer_event_report_data*) malloc(sizeof(struct glucometer_event_report_data));

    time(&now);
    localtime_r(&now, &nowtm);

    data->capillary_whole_blood = 10.2 + random() % 20;

    data->century = nowtm.tm_year / 100 + 19;
    data->year = nowtm.tm_year % 100;
    data->month = nowtm.tm_mon + 1;
    data->day = nowtm.tm_mday;
    data->hour = nowtm.tm_hour;
    data->minute = nowtm.tm_min;
    data->second = nowtm.tm_sec;
    data->sec_fractions = 50;

    return data;
}

/**
 * Generate data for Thermometer event report
 */
//Created for Castalia
void *thermometer_event_report_cb()
{
    time_t now;
    struct tm nowtm;
    struct thermometer_event_report_data* data =
        (thermometer_event_report_data*) malloc(sizeof(struct thermometer_event_report_data));

    time(&now);
    localtime_r(&now, &nowtm);

    data->temperature = 36.5 + random() % 5;

    data->century = nowtm.tm_year / 100 + 19;
    data->year = nowtm.tm_year % 100;
    data->month = nowtm.tm_mon + 1;
    data->day = nowtm.tm_mday;
    data->hour = nowtm.tm_hour;
    data->minute = nowtm.tm_min;
    data->second = nowtm.tm_sec;
    data->sec_fractions = 50;

    return data;
}

/**
 * Generate data for Basic ECG event report
 */
//Created for Castalia
void *basic_ECG_event_report_cb()
{
    time_t now;
    struct tm nowtm;
    struct basic_ECG_event_report_data* data =
        (basic_ECG_event_report_data*) malloc(sizeof(struct basic_ECG_event_report_data));

    time(&now);
    localtime_r(&now, &nowtm);
    double R;
    intu16 X;
    data->mV = (intu8*) calloc(160, sizeof(intu8));
    //80 samples
    for (int i = 0; i < 80; i++)
    {
        R = ECG_samples[i];
        X = (intu16)((R - B) / M);
        (*((uint16_t *) (data->mV + (i*2)))) = htons(X);
    }

    data->century = nowtm.tm_year / 100 + 19;
    data->year = nowtm.tm_year % 100;
    data->month = nowtm.tm_mon + 1;
    data->day = nowtm.tm_mday;
    data->hour = nowtm.tm_hour;
    data->minute = nowtm.tm_min;
    data->second = nowtm.tm_sec;
    data->sec_fractions = 50;

    return data;
}

/**
 * Generate data for MDS
 */
struct mds_system_data *mds_data_cb()
{
    struct mds_system_data* data = (mds_system_data*) malloc(sizeof(struct mds_system_data));
    std::memcpy(&data->system_id, AGENT_SYSTEM_ID_VALUE, 8);
    return data;
}

//int timer_count_timeout(Context *ctx)
//{
//return 1;
//}

//void timer_reset_timeout(Context *ctx)
//{
//}

//void device_associated(Context *ctx)
//{
//fprintf(stderr, " main: Associated\n");
//alarm(3);
//}

//void device_unavailable(Context *ctx)
//{
//fprintf(stderr, " main: Disasociated\n");
//alarms = 0;
//}

//void device_connected(Context *ctx, const char *addr)
//{
//fprintf(stderr, "main: Connected\n");

//// ok, make it proceed with association
//// (agent has the initiative)
//agent_associate(ctx->id);
//}

//void sigalrm(int dummy)
//{
//// This is not incredibly safe, because signal may interrupt
//// processing, and is not a technique for a production agent,
//// but suffices for this quick 'n dirty sample

//if (alarms > 1) {
//agent_send_data(CONTEXT_ID);
//alarm(3);
//} else if (alarms == 1) {
//agent_request_association_release(CONTEXT_ID);
//alarm(3);
//} else {
//agent_disconnect(CONTEXT_ID);
//}

//--alarms;
//}

///**
//* Configure application to use tcp plugin
//*/
//void tcp_mode()
//{
//int port = 6024;
//// NOTE we know that plugin id = 1 here, but
//// might not be the case!
//CONTEXT_ID.plugin = 1;
//CONTEXT_ID.connid = port;
//plugin_network_tcp_agent_setup(&comm_plugin, port);
//}


