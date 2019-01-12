#ifndef _MANAGER_H_
#define _MANAGER_H_

extern "C" {
#include "communication/plugin/plugin.h"
#include "communication/communication.h"
#include "communication/context_manager.h"
#include "communication/context.h"
#include "ieee11073.h"
#include "manager.h"
}

#include "VirtualApplication.h"
#include "MyPacket_m.h"
#include "m_plugin_castalia.h"
#include "m_global.h"
#include "Agent.h"

using namespace std;

class Manager : public VirtualApplication
{

private:
    double packet_rate;
    double startupDelay;
    double delayLimit;
    float packet_spacing;
    int recipientId;
    int totalPacketsReceived;
    string recipientAddress;
    unsigned int my_plugin_number;
    unsigned int numPlugin;
    unsigned int numNodes;

    //variables below are used to determine the packet delivery rates.
    map<long,int> packetsReceived;
    map<long,int> bytesReceived;
    map<long,int> packetsSent;

    map<long,int> dataSN;//data sequence number
    map<long,int> last_packet;//control duplicate packets

protected:
    void startup();
    void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
    void handleRadioControlMessage(RadioControlMessage *);
    void timerFiredCallback(int);
    void finishSpecific();
    MyPacket* createDataPacket(int seqNum);
    void retransmitPacket(int nodeNumber);

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

};

#endif
