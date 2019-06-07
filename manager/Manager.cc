#include "Manager.h"

Define_Module(Manager);

//extern variable declared in m_global.h
m_Tmsg *m_st_msg = NULL;
CommunicationPlugin *m_comm_plugin;
int *m_SETTIMER = NULL;

void Manager::startup()
{
    packet_rate = par("packet_rate");
    startupDelay = par("startupDelay");
    delayLimit = par("delayLimit");
    //managerInitiated = par("managerInitiated");
    //numberOfReceivedMeasurementsToSendStop = par("numberOfReceivedMeasurementsToSendStop");
    totalPacketsReceived = 0;
    numNodes = getParentModule()->getParentModule()->par("numNodes");
    //fprintf(stderr,"\n entrei init \n");
    for (unsigned int i = 1; i < numNodes; i++)
    {
        setIsManagerInitiatedModeActive(false, i);
        setIsNumberOfReceivedMeasurementsToSendStop(false, i);
        setNumberOfReceivedMeasurementsToSendStop(0, i);
    }

    cTopology *topo; // temp variable to access packets received by other nodes
    topo = new cTopology("topo");
    topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector()); //Extracts model topology by the fully qualified NED type name of the modules.

    // cModule *targetModule = getParentModule()->getSubmodule("foo");
    // Foo * = check_and_cast<Foo *>(targetModule);

    for (unsigned int i = 1; i < numNodes; i++)
    {
        Agent *appModule = check_and_cast<Agent *>(topo->getNode(i)->getModule()->getSubmodule("Application"));
        if (appModule)
        {
            //access the agent managerInitiated parameter
            if (appModule->par("managerInitiated"))
            {
                setIsManagerInitiatedModeActive(true, i);
                string managerInitiatedMode = appModule->par("managerInitiateMode").stringValue();
                if (!(strcmp(managerInitiatedMode.c_str(), "noTimePeriodMode")))
                {
                    setIsNumberOfReceivedMeasurementsToSendStop(true, i);
                    double temp = appModule->par("numberOfReceivedMeasurementsToSendStop");
                    setNumberOfReceivedMeasurementsToSendStop(temp, i);
                }
            }
        }
    }
    delete (topo);

    //creates a Tmsg struct for each node
    if (m_st_msg == NULL)
        m_st_msg = new m_Tmsg[numNodes];

    //creates a SETTIMER for each agent
    if (m_SETTIMER == NULL)
    {
        m_SETTIMER = new int[numNodes];
        for (unsigned int i = 0; i < numNodes; i++)
        {
            m_SETTIMER[i] = 0;
        }
    }

    //creates a m_com_plugin for each agent
    if (m_comm_plugin == NULL)
    {
        m_comm_plugin = new CommunicationPlugin[numNodes];
        for (unsigned int i = 0; i < numNodes; i++)
        {
            m_comm_plugin[i] = COMMUNICATION_PLUGIN_NULL;
        }
    }

    /**
     * Get the total number of plugins, the manger has one
     * plugin for each agent. The number of each plugin
     * in manager has to be always even numbers.
     * */
    numPlugin = (2 * (numNodes - 1));

    ManagerListener listener[numNodes];
    for (unsigned int i = 0; i < numNodes; i++)
    {
        listener[i] = MANAGER_LISTENER_EMPTY;
    }

    /**
     * Initialize the plugins. OBS: manager uses only
     * even plugins. The odds plugins are from agents
     * */
    for (unsigned int i = 2; i <= numPlugin; i += 2)
    {
        if ((i % 2) == 0)
        {
            //node number
            unsigned int nodeId = i / 2;
            m_comm_plugin[nodeId] = communication_plugin();
            m_castalia_mode(nodeId);
            //fprintf(stderr, "\nIEEE 11073 Sample application\n");
            DEBUG("\nIEEE 11073 Sample application\n");
            m_comm_plugin[nodeId].timer_count_timeout = m_timer_count_timeout;
            m_comm_plugin[nodeId].timer_reset_timeout = m_timer_reset_timeout;

            CommunicationPlugin *m_comm_plugins[] = {&m_comm_plugin[nodeId], 0};
            m_CONTEXT_ID = {i, 0};
            manager_init(m_CONTEXT_ID, m_comm_plugins);

            listener[nodeId].measurement_data_updated = &new_data_received;
            listener[nodeId].device_available = &device_associated;
            listener[nodeId].device_unavailable = &m_device_unavailable;

            manager_add_listener(m_CONTEXT_ID, listener[nodeId]);
            manager_start(m_CONTEXT_ID);
        }
    }

    //variables used in finishSpecif
    packetsSent.clear();
    packetsReceived.clear();
    bytesReceived.clear();

    /**
     * Clean up the controlPackets and MeasurementPacket
     * map variables from m_plugin_castalia.cc file
     * */
    m_clearVarMap();

    //data sequence number
    dataSN.clear();
    //control duplicate packets
    last_packet.clear();

    declareOutput("Packets received per node");
}

void Manager::fromNetworkLayer(ApplicationPacket *rcvPacketa,
                               const char *source, double rssi, double lqi)
{
    //fprintf(stderr,"\nchegou\n");
    /**
     * Converte the packet from ApplicationPacket
     * to MyPacket.
     * */
    MyPacket *rcvPacket = check_and_cast<MyPacket *>(rcvPacketa);

    int sequenceNumber = rcvPacket->getSequenceNumber();

    unsigned int sourceId = atoi(source);

    /**
     * The reason to have the vars recipientAddress and
     * recipientId (both with global scope) is to use
     * them outside of this function
     * */
    recipientAddress = source;
    recipientId = sourceId;
    //fprintf(stderr,"\naiaiaiaia\n");
    if (sequenceNumber != last_packet[sourceId])
    {
        //fprintf(stderr,"\nagora vai\n");
        //Checks if there is a activated timeout for some agent
        Context *ctx;

        my_plugin_number = sourceId * 2;
        //fprintf(stderr,"\nachei\n");
        m_CONTEXT_ID = {my_plugin_number, 0};

        ctx = context_get_and_lock(m_CONTEXT_ID);

        fsm_states c = ctx->fsm->state;
        switch (c)
        {
        case fsm_state_operating:
        {
            if (getTimer(sourceId) != 0)
            {
                cancelTimer(sourceId);
            }
            break;
        }
        default:
            break;
        }
        context_unlock(ctx);

        last_packet[sourceId] = sequenceNumber;

        //fprintf(stderr,"\naiaiaiaia\n");

        //clear the Tmsg struct to receive a new packet
        m_st_msg[sourceId].tam_buff = 0;
        m_st_msg[sourceId].buff_msgRep.clear();

        while (!m_st_msg[sourceId].msgType.empty())
            m_st_msg[sourceId].msgType.pop();

        m_st_msg[sourceId].tam_buff = rcvPacket->getTam_buff();

        for (int i = 0; i < m_st_msg[sourceId].tam_buff; i++)
        {
            m_st_msg[sourceId].buff_msgRep.push_back(rcvPacket->getBuff(i));
        }

        if ((strcmp(source, SELF_NETWORK_ADDRESS)) != 0)
        {
            if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit)
            {

                trace() << "Received packet #" << sequenceNumber << " from node " << source;
                collectOutput("Packets received per node", sourceId); //Adds one to the value of output name with index "sourceId".
                packetsReceived[sourceId]++;
                bytesReceived[sourceId] += rcvPacket->getByteLength();
                totalPacketsReceived++;
                //collectHistogram("Application level latency, total",1000 * (simTime() - rcvPacket->getCreationTime()).dbl());

                if (m_st_msg[sourceId].tam_buff > 0)
                {

                    m_st_msg[sourceId].buff_msgSed.clear();

                    m_CONTEXT_ID = {my_plugin_number, 0};
                    Context *m_ctx;
                    m_ctx = context_get_and_lock(m_CONTEXT_ID);

                    //receive all the messages in received packet
                    while ((communication_wait_for_data_input(m_ctx)) == NETWORK_ERROR_NONE)
                    {
                        while (!m_st_msg[sourceId].msgType.empty())
                            m_st_msg[sourceId].msgType.pop();
                        communication_read_input_stream(m_ctx->id);
                        trace() << "type: " << m_st_msg[sourceId].msgType.front();
                        m_st_msg[sourceId].msgType.pop();
                    }

                    // if(!m_st_msg[sourceId].msgType.empty())
                    // {
                    //     trace() << "type: " << m_st_msg[sourceId].msgType.front();
                    //     m_st_msg[sourceId].msgType.pop();
                    // }

                    //an error occurred, cancel timers for the current node
                    if (m_ctx->fsm->state == fsm_state_unassociated)
                        cancelTimer(sourceId);

                    if (m_SETTIMER[sourceId])
                    {
                        setTimer(sourceId, 3);
                        m_SETTIMER[sourceId] = 0;
                    }

                    if (m_st_msg[sourceId].tam_buff > 0)
                    {

                        dataSN[sourceId]++;
                        trace() << "Sending packet #" << dataSN[sourceId] << " to node " << sourceId; //sequence number
                        while (!m_st_msg[sourceId].msgType.empty())
                        {
                            trace() << "type: " << m_st_msg[sourceId].msgType.front();
                            m_st_msg[sourceId].msgType.pop();
                        }
                        toNetworkLayer(createDataPacket(dataSN[sourceId]), source);
                        packetsSent[recipientId]++;
                    }
                    context_unlock(m_ctx);
                }
            }
            else
            {
                trace() << "Packet #" << sequenceNumber << " from node " << source << " exceeded delay limit of " << delayLimit << "s";
            }
        }
        else
        {
            ApplicationPacket *fwdPacket = rcvPacket->dup();
            // Reset the size of the packet, otherwise the app overhead will keep adding on
            fwdPacket->setByteLength(0);
            toNetworkLayer(fwdPacket, recipientAddress.c_str());
        }
    }
    //retransmit the last packet sent
    else if (sequenceNumber == last_packet[sourceId])
    {
        if (m_st_msg[sourceId].tam_buff > 0)
        {
            my_plugin_number = sourceId * 2;
            m_CONTEXT_ID = {my_plugin_number, 0};
            retransmitPacket(sourceId);
        }
    }
}

void Manager::timerFiredCallback(int index)
{
    Context *ctx;
    //plugins for manager is always even numbers
    my_plugin_number = index * 2;
    m_CONTEXT_ID = {my_plugin_number, 0};
    ctx = context_get_and_lock(m_CONTEXT_ID);
    fsm_states c = ctx->fsm->state;
    switch (c)
    {
    case fsm_state_operating:
    {
        trace() << "response not received for packet #" << dataSN[index] << " from node " << index << " in operating mode "; //sequence number
        break;
    }
    case fsm_state_associating:
    {
        trace() << "response not received for packet #" << dataSN[index] << " from node " << index << " in associating mode "; //sequence number
        break;
    }
    default:
        break;
    }

    m_st_msg[index].buff_msgSed.clear();
    m_st_msg[index].tam_buff = 0;
    while (!m_st_msg[index].msgType.empty())
        m_st_msg[index].msgType.pop();

    manager_request_association_abort(m_CONTEXT_ID);
    const char *recipient = std::to_string(index).c_str();
    dataSN[index]++;
    trace() << "Sending packet #" << dataSN[index] << " to node " << index; //sequence number
    while (!m_st_msg[index].msgType.empty())
    {
        trace() << "type: " << m_st_msg[index].msgType.front();
        m_st_msg[index].msgType.pop();
    }
    toNetworkLayer(createDataPacket(dataSN[index]), recipient);
    packetsSent[recipientId]++;

    context_unlock(ctx);
}

// This method processes a received carrier sense interupt. Used only for demo purposes
// in some simulations. Feel free to comment out the trace command.
void Manager::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
    switch (radioMsg->getRadioControlMessageKind())
    {
    case CARRIER_SENSE_INTERRUPT:
        trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
        break;
    }
}

void Manager::finishSpecific()
{
    declareOutput("Packets reception rate");
    declareOutput("Packets loss rate");
    declareOutput("Measurement Packets Received");

    cTopology *topo; // temp variable to access packets received by other nodes
    topo = new cTopology("topo");
    topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector()); //Extracts model topology by the fully qualified NED type name of the modules.

    //long bytesDelivered = 0;
    for (unsigned int i = 0; i < numNodes; i++)
    {
        if (i == 0)
        {
            Manager *appModule = dynamic_cast<Manager *>(topo->getNode(i)->getModule()->getSubmodule("Application"));
            if (appModule)
            {
                int packetsSent = appModule->getPacketsSent(self);
                if (packetsSent > 0) // this node sent us some packets
                {
                    float rate = (float)packetsReceived[i] / packetsSent;
                    collectOutput("Packets reception rate", i, "total", rate);
                    collectOutput("Packets loss rate", i, "total", 1 - rate);
                }
                //bytesDelivered += appModule->getBytesReceived(self);
            }
        }
        else
        {
            Agent *appModule = dynamic_cast<Agent *>(topo->getNode(i)->getModule()->getSubmodule("Application"));
            if (appModule)
            {
                int packetsSent = appModule->getPacketsSent(self);
                if (packetsSent > 0) // this node sent us some packets
                {
                    float rate = (float)packetsReceived[i] / packetsSent;
                    collectOutput("Packets reception rate", i, "total", rate);
                    collectOutput("Packets loss rate", i, "total", 1 - rate);
                }
                //bytesDelivered += appModule->getBytesReceived(self);
            }
        }
        //the manger does not received measurement from himself
        if (i != (unsigned int)atoi(SELF_NETWORK_ADDRESS))
            collectOutput("Measurement Packets Received", i, "", m_getMeasurementPacketsTotal(i));
    }

    delete (topo);
    long bytesDelivered = getBytesReceived(atoi(SELF_NETWORK_ADDRESS)); 
    if (bytesDelivered > 0)
    {
        double energy = (resMgrModule->getSpentEnergy() * 1000000000) / (bytesDelivered * 8); //in nanojoules/bit
        declareOutput("Energy nJ/bit");
        collectOutput("Energy nJ/bit", "", energy);
    }
    manager_finalize(m_CONTEXT_ID);

    if (m_st_msg != NULL)
    {
        delete[] m_st_msg;
        m_st_msg = NULL;
    }

    if (m_comm_plugin != NULL)
    {
        delete[] m_comm_plugin;
        m_comm_plugin = NULL;
    }

    if (m_SETTIMER != NULL)
    {
        delete[] m_SETTIMER;
        m_SETTIMER = NULL;
    }
}

MyPacket *Manager::createDataPacket(int seqNum)
{
    MyPacket *pkt = new MyPacket("mypacket", APPLICATION_PACKET);
    int nodeNumber = my_plugin_number / 2;
    //size of buff
    pkt->setBuffArraySize(m_st_msg[nodeNumber].tam_buff);
    for (int i = 0; i < m_st_msg[nodeNumber].tam_buff; i++)
    {
        pkt->setBuff(i, m_st_msg[nodeNumber].buff_msgSed[i]);
    }
    pkt->setTam_buff(m_st_msg[nodeNumber].tam_buff);
    pkt->setSequenceNumber(seqNum);
    pkt->setByteLength(pkt->getBuffArraySize() + sizeof(m_st_msg[nodeNumber].tam_buff));
    return pkt;
}

void Manager::retransmitPacket(int nodeNumber)
{
    trace() << "resending packet #" << dataSN[nodeNumber] << " to node " << nodeNumber; //sequence number
    toNetworkLayer(createDataPacket(dataSN[nodeNumber]), recipientAddress.c_str());
    packetsSent[recipientId]++;
    //collectOutput("Number of transmissions retries per packet", dataSN[nodeNumber]);//Adds one to the value of output name with index 3.
}
