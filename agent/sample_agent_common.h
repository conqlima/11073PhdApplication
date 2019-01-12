#ifndef SAMPLE_AGENT_COMMON_H_
#define SAMPLE_AGENT_COMMON_H_

//#include "agent.h"

struct mds_system_data;
extern intu8 AGENT_SYSTEM_ID_VALUE[];

/**See Scale and range specification example, pg 151, Annex B, from Optimized Exchange Protocol 20601 */
/**
 * M = (upper-absolute-value – lower-absolute-value) / (upper-scaled-value – lower-scaled-value)
 * B = upper-absolute-value – (M × upper-scaled-value)
 * */

/**double M = (2.0-(-2.0))/(800.0-0.0) = 0,005*/
const double M = 0.005;
/**double B = 2.0-(M*800.0) = -2*/
const double B = -2.000;

/**
 * The number of samples can't be changed.
 * If you really need another number of samples,
 * you must change the size of the message in file
 * basic_ECG.c, change the loop in json_encoder.c
 * function sa_convert_scaled_values2absolute() for the
 * new size. It's recommended not change the number of samples,
 * keep exactly 80 values, but the samples may be changed.
**/
static const double ECG_samples[] =
{
    -0.060,//388
    -0.065,//387
    -0.060,//388
    -0.075,//385
    -0.065,//387
    -0.070,//386
    -0.070,//386
    -0.090,
    -0.080,
    -0.095,
    -0.080,
    -0.095,
    -0.080,
    -0.095,
    -0.085,
    -0.090,
    -0.090,
    -0.100,
    -0.085,
    -0.105,//19º value
    -0.090,
    -0.045,
    0.005,//0.000 is received
    0.015,
    0.045,
    0.155,
    0.140,
    0.045,
    0.005,
    -0.040,
    -0.085,
    -0.200,
    -0.195,
    -0.200,
    -0.200,
    -0.240,
    -0.130,
    0.340,
    1.155,
    1.470,//39º value
    -0.155,
    -0.825,
    -0.590,
    -0.350,
    -0.155,
    -0.170,
    -0.140,
    -0.155,
    -0.115,
    -0.125,
    -0.090,
    -0.095,
    -0.065,
    -0.055,
    -0.015,
    -0.005,
    0.035,
    0.045,
    0.090,
    0.110,//59º value
    0.150,
    0.180,
    0.205,
    0.225,
    0.230,
    0.220,
    0.235,
    0.230,
    0.200,
    0.170,
    0.120,
    0.075,
    0.040,
    0.020,
    0.005,
    -0.005,
    -0.005,
    -0.010,
    -0.015,
    -0.010
};//80 values, 79º value

void *oximeter_event_report_cb();
void *blood_pressure_event_report_cb();
void *weightscale_event_report_cb();
void *glucometer_event_report_cb();
void *thermometer_event_report_cb();
void *basic_ECG_event_report_cb();

//void timer_reset_timeout(Context *ctx);
//int timer_count_timeout(Context *ctx);
//void device_connected(Context *ctx, const char *addr);
//void device_unavailable(Context *ctx);
//void device_associated(Context *ctx);
//void sigalrm(int dummy);
//void tcp_mode();

struct mds_system_data *mds_data_cb();

#endif
