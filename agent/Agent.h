#ifndef _AGENT_H_
#define _AGENT_H_

extern "C" {
#include "communication/agent_ops.h"
#include "communication/plugin/plugin.h"
#include "communication/communication.h"
#include "communication/context_manager.h"
#include "communication/context.h"
#include "specializations/pulse_oximeter.h"
#include "specializations/blood_pressure_monitor.h"
#include "specializations/weighing_scale.h"
#include "specializations/glucometer.h"
#include "specializations/thermometer.h"
#include "specializations/basic_ECG.h"
#include "ieee11073.h"
#include "agent.h"
#include "manager.h"
#include "util/log.h"
}

#include "VirtualApplication.h"
#include "MyPacket_m.h"
#include "plugin_castalia.h"
#include "global.h"
#include "sample_agent_common.h"
#include "Manager.h"

using namespace std;

/**
 * Number of retransmissions tries in associaton mode.
 * See section: 8.4.3 Timeout variables,
 * Optimized exchange protocol - 2016, page 75.
 */
#define RC_COUNT 3
#define ALFA 0.125
#define BETA 0.25

enum AgentTimers
{
    SEND_PACKET = 1,
    TO_ASSOC,
    TO_OPERA
};

class Agent : public VirtualApplication
{

private:
    double packet_rate;//not used
    double startupDelay;
    double delayLimit;
    double reading_rate;
    double maxSimTime;
    double timeOutToRetransmitPacket;
    double managerInitiatedTime;
    simtime_t initialTime;
    //double initialTime;
    //double numberOfReceivedMeasurementsToSendStop;
    double dinamicTimeoutMean;
    double EstimatedRTT;
    double DevRTT;
    float packet_spacing;
    float data_spacing;
    int dinamicTimeoutMeanCount;
    int dataSN;
    int recipientId;
    int alarmt;
    int last_packet;
    int specialization;
    int RC;
    int numNodes;
    int numOfRetransmissions;
    int maxNumOfRetransmition;
    int isTheFirstAssociation;
    bool dinamicTimeout;
    bool confirmed_event;
    bool retransmissionPacket;
    bool managerInitiated;
    bool errorOccurred;
    unsigned int my_plugin_number;
    unsigned int opt;
    unsigned int nodeNumber;
    string recipientAddress;
    string application_name;
    string managerInitiatedMode;
    void* (*event_report_cb)();
    //MyPacket *pktGlobal;

    //variables below are used to determine the packet delivery rates.
    map<long,int> packetsReceived;
    map<long,int> bytesReceived;
    map<long,int> packetsSent;

protected:
    void startup();
    void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
    void handleRadioControlMessage(RadioControlMessage *);
    void timerFiredCallback(int);
    void finishSpecific();
    MyPacket* createDataPacket(int seqNum);
    void tryNewAssociationForTimeout(void);
    void tryNewAssociationForAbort(void);
    void retransmitPacket(void);
    void updateTimeOutToRetransmitPacket(double);

public:
    int getPacketsSent(int addr)
    {
        return packetsSent[addr];
    }
    int getPacketsReceived(int addr)
    {
        return packetsReceived[addr];
    }
    int getBytesReceived(int addr)
    {
        return bytesReceived[addr];
    }
    int getNumberOfNodes()
    {
        return getParentModule()->getParentModule()->par("numNodes");
    }

};

#endif
