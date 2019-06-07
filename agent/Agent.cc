#include "Agent.h"

Define_Module(Agent);

//this struct represent all the msg exchanged between nodes
Tmsg *st_msg = NULL;

//this struct contains all information about an agent
CommunicationPlugin *comm_plugin = NULL;

//var used by Antidote stack library to set 1 if timeout is required 0 otherwise.
int *SETTIMER = NULL;

/*
 * Var set by user in .ini file to define the hub node.
 * The hub node has to be always the MANAGER.
 */
int HUBNODE;

void Agent::startup()
{
    HUBNODE = par("hubNode");
    retransmissionPacket = par("retransmitPacket");
    timeOutToRetransmitPacket = par("timeOutToRetransmitPacket");
    maxNumOfRetransmition = par("maxNumOfRetransmition");
    confirmed_event = par("confirmed_event");
    packet_rate = par("packet_rate");
    managerInitiated = par("managerInitiated");
    managerInitiatedMode = par("managerInitiateMode").stringValue();
    managerInitiatedTime = par("managerInitiatedTime");

    //Node 0 is always the next recipient
    recipientAddress = par("nextRecipient").stringValue();
    recipientId = atoi(recipientAddress.c_str());

    startupDelay = par("startupDelay");
    delayLimit = par("delayLimit");
    application_name = par("application_type").stringValue();
    packet_spacing = packet_rate > 0 ? 1 / float(packet_rate) : -1;
    dataSN = 0;
    reading_rate = par("measurements_per_second");
    RC = RC_COUNT;
    last_packet = -1;
    numOfRetransmissions = 0;

    //Get the node number
    nodeNumber = atoi(SELF_NETWORK_ADDRESS);

    //Get the sim-time-limit value from .ini file
    cConfigOption simTimeConfig("sim-time-limit", true, cConfigOption::Type::CFG_DOUBLE, "s", "300", "");
    maxSimTime = cSimulation::getActiveSimulation()->getEnvir()->getConfig()->getAsDouble(&simTimeConfig);

    //startup delay is not part of simulations
    maxSimTime = maxSimTime - startupDelay;
    //The manager initiate sending measurements
    communication_agent_setIsTheStartMode(0, nodeNumber);
    if (managerInitiated)
    {
        alarmt = -1;

        if (!strcmp(managerInitiatedMode.c_str(), "singleMode"))
        {
            //single mode choose
            DataReqMode mode = DATA_REQ_START_STOP | DATA_REQ_SUPP_SCOPE_CLASS | DATA_REQ_SUPP_MODE_SINGLE_RSP;
            manager_setDataReqMode(mode, nodeNumber);
        }
        else if (!strcmp(managerInitiatedMode.c_str(), "timePeriodMode"))
        {
            //time period mode choose
            DataReqMode mode = DATA_REQ_START_STOP | DATA_REQ_SUPP_SCOPE_CLASS | DATA_REQ_SUPP_MODE_TIME_PERIOD;
            manager_setDataReqMode(mode, nodeNumber);
        }
        else if (!strcmp(managerInitiatedMode.c_str(), "noTimePeriodMode"))
        {
            //no time period mode choose
            DataReqMode mode = DATA_REQ_START_STOP | DATA_REQ_SUPP_SCOPE_CLASS | DATA_REQ_SUPP_MODE_TIME_NO_LIMIT;
            manager_setDataReqMode(mode, nodeNumber);
        }
        else
        {
            throw cRuntimeError("Invalid managerInitiatedMode value: %s", managerInitiatedMode.c_str());
        }
    }
    else//agent initiated mode
    {
        //set the 16º bit
        DataReqMode mode = DATA_REQ_START_STOP | DATA_REQ_SUPP_SCOPE_CLASS | DATA_REQ_SUPP_MODE_SINGLE_RSP;
        manager_setDataReqMode(mode, nodeNumber);

        //retransmission mode use confirmed events
        if(retransmissionPacket)
            confirmed_event = true;

        //Agent initiate the transmition of packets
        alarmt = (int)(maxSimTime * reading_rate);
    }
    //Calculate the time spacing between measurements
    data_spacing = reading_rate > 0 ? 1.0 / reading_rate : -1; //divide 1s para a taxa de pacotes para saber o espaçamento entre cada transmissão

    //Get the total number of nodes
    numNodes = getNumberOfNodes();

    //Creates a Tmsg struct vector
    if (st_msg == NULL)
        st_msg = new Tmsg[numNodes];

    //creates a SETTIMER vector
    if (SETTIMER == NULL)
        SETTIMER = new int[numNodes];

    if (!(strcmp(application_name.c_str(), "pulseoximeter")))
    {
        opt = 1;
    }
    else if (!(strcmp(application_name.c_str(), "bloodpressure")))
    {
        opt = 2;
    }
    else if (!(strcmp(application_name.c_str(), "weightscale")))
    {
        opt = 3;
    }
    else if (!(strcmp(application_name.c_str(), "glucometer")))
    {
        opt = 4;
    }
    else if (!(strcmp(application_name.c_str(), "thermometer")))
    {
        opt = 5;
    }
    else if (!(strcmp(application_name.c_str(), "basicECG")))
    {
        opt = 6;
    }
    else
    {
        throw cRuntimeError("Invalid application name in node %s", SELF_NETWORK_ADDRESS);
    }

    //All SETTIMER has to initialize with 0
    SETTIMER[nodeNumber] = 0;

    //Initialize all comm_plugin positions
    if (comm_plugin == NULL)
    {
        comm_plugin = new CommunicationPlugin[numNodes];
        for (int i = 0; i < numNodes; i++)
        {
            comm_plugin[i] = COMMUNICATION_PLUGIN_NULL;
        }
    }
    comm_plugin[nodeNumber] = communication_plugin();

    //Set Castalia framework to communicate with Antidote Stack
    castalia_mode(nodeNumber);

    //The callback function for timeouts
    comm_plugin[nodeNumber].timer_count_timeout = timer_count_timeout;
    comm_plugin[nodeNumber].timer_reset_timeout = timer_reset_timeout;

    CommunicationPlugin *plugins[] = {&comm_plugin[nodeNumber], 0};

    if (opt == 2) /* Blood Pressure */
    {
        //fprintf(stderr, "Starting Blood Pressure Agent\n");
        DEBUG("Starting Blood Pressure Agent\n");
        event_report_cb = blood_pressure_event_report_cb;
        specialization = 0x02BC;
        if (confirmed_event)
            event_conf_or_unconf_blood_pressure = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
    }
    else if (opt == 3) /* Weight Scale */
    {
        //fprintf(stderr, "Starting Weight Scale Agent\n");
        DEBUG("Starting Weight Scale Agent\n");
        event_report_cb = weightscale_event_report_cb;
        specialization = 0x05DC;
        if (confirmed_event)
            event_conf_or_unconf_weighting_scale = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
    }
    else if (opt == 4) /* Glucometer */
    {
        //fprintf(stderr, "Starting Glucometer Agent\n");
        DEBUG("Starting Glucometer Agent\n");
        event_report_cb = glucometer_event_report_cb;
        specialization = 0x06A4;
        if (confirmed_event)
            event_conf_or_unconf_glucometer = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
    }
    else if (opt == 5) /* Thermometer */
    {
        //fprintf(stderr, "Starting Thermometer Agent\n");
        DEBUG("Starting Thermometer Agent\n");
        event_report_cb = thermometer_event_report_cb;
        specialization = 0x0320;
        if (confirmed_event)
            event_conf_or_unconf_thermometer = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
    }
    else if (opt == 6) /* basic ECG */
    {
        //fprintf(stderr, "Starting Basic ECG Agent\n");
        DEBUG("Starting Basic ECG Agent\n");
        event_report_cb = basic_ECG_event_report_cb;
        //specialization = 0x0258;
        specialization = 0x4000;
        if (confirmed_event)
            event_conf_or_unconf_basic_ecg = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
    }
    else /* Default Pulse Oximeter */
    {
        //fprintf(stderr, "Starting Pulse Oximeter Agent\n");
        DEBUG("Starting Pulse Oximeter Agent\n");
        event_report_cb = oximeter_event_report_cb;
        // change to 0x0191 if you want timestamps
        specialization = 0x0190;
        if (confirmed_event)
            event_conf_or_unconf_pulse_oximeter = ROIV_CMIP_CONFIRMED_EVENT_REPORT_CHOSEN;
    }

    /**
     * Initializes my_plugin_number with the agent plugin id.
     * The plugin id of agents are ALWAYS ODD numbers.
     * e.g: The node 3 has the plugin id 5:
     *  -- nodeNumber * 2 - 1 = plugin id --
     * 3 * 2 - 1 = 5
     * */
    if (nodeNumber == 1)
    {
        CONTEXT_ID = {1, 0};
        my_plugin_number = CONTEXT_ID.plugin;
    }
    else
    {
        CONTEXT_ID = {(nodeNumber * 2) - 1, 0};
        my_plugin_number = CONTEXT_ID.plugin;
    }

    //Initialize the agent
    agent_init(CONTEXT_ID, plugins, specialization, event_report_cb, mds_data_cb);
    /**
     * Callbacks functions to notify the user when
     * the agents changes de finite states machine
     * */
    AgentListener listener = AGENT_LISTENER_EMPTY;
    listener.device_connected = &device_connected;
    listener.device_associated = &device_associated;
    listener.device_unavailable = &device_unavailable;

    agent_add_listener(CONTEXT_ID, listener);

    /**
     * Clean up the controlPackets and MeasurementPacket
     * map variables from plugin_castalia.cc file
     * */
    clearVarMap();

    //Agent takes the iniciative to associate
    if (data_spacing != -1)
    {
        agent_start(CONTEXT_ID);
        dataSN++;
        setTimer(SEND_PACKET, 0 + startupDelay);

        //Number of associations made
        isTheFirstAssociation = 0;

        //Variables used in finishSpecific
        packetsSent.clear();
        packetsReceived.clear();
        bytesReceived.clear();

        declareOutput("Packets received per node");
        declareHistogram("Number of transmissions' retries per packet", 0, 10, 10);
        //declareHistogram("Application level latency, total", 0, 3500, 35);
    }
    else
    {
        throw cRuntimeError("Invalid reading_rate value: %d", reading_rate);
    }
}

void Agent::fromNetworkLayer(ApplicationPacket *rcvPacketa,
                             const char *source, double rssi, double lqi)
{
    //Converte the packet from ApplicationPacket to MyPacket
    MyPacket *rcvPacket = check_and_cast<MyPacket *>(rcvPacketa);

    int sequenceNumber = rcvPacket->getSequenceNumber();

    int sourceId = atoi(source);

    if (sequenceNumber != last_packet)
    {
        //Checks if there is a activated timeout
        Context *ctx;
        CONTEXT_ID = {my_plugin_number, 0};
        ctx = context_get_and_lock(CONTEXT_ID);
        fsm_states c = ctx->fsm->state;
        switch (c)
        {
        case fsm_state_associating:
        {
            if (getTimer(TO_ASSOC) != 0)
                cancelTimer(TO_ASSOC);
            break;
        }

        case fsm_state_operating:
        {
            //collectHistogram("Application level latency, total",1000 * (simTime() - rcvPacket->getCreationTime()).dbl());
            if (getTimer(TO_OPERA) != 0)
                cancelTimer(TO_OPERA);
            break;
        }
        default:
            break;
        }
        context_unlock(ctx);

        //clear the Tmsg struct to receive a new packet
        st_msg[nodeNumber].tam_buff = 0;
        st_msg[nodeNumber].buff_msgRep.clear();
        while (!st_msg[nodeNumber].msgType.empty())
            st_msg[nodeNumber].msgType.pop();

        //Updates the sequence number
        last_packet = sequenceNumber;

        st_msg[nodeNumber].tam_buff = rcvPacket->getTam_buff();

        for (int i = 0; i < st_msg[nodeNumber].tam_buff; i++)
        {
            st_msg[nodeNumber].buff_msgRep.push_back(rcvPacket->getBuff(i));
        }

        if ((strcmp(source, SELF_NETWORK_ADDRESS)) != 0)
        {
            if (delayLimit == 0 || (simTime() - rcvPacket->getCreationTime()) <= delayLimit)
            {
                trace() << "Received packet #" << sequenceNumber << " from node " << source;

                //Adds one to the value of output name with sourceId.
                collectOutput("Packets received per node", sourceId);
                packetsReceived[sourceId]++;
                bytesReceived[sourceId] += rcvPacket->getByteLength();

                CONTEXT_ID = {my_plugin_number, 0};
                ctx = context_get_and_lock(CONTEXT_ID);

                st_msg[nodeNumber].buff_msgSed.clear();

                //Checks if the received packet is not empty
                if (st_msg[nodeNumber].tam_buff > 0)
                {
                    //receive all the messages
                    while ((communication_wait_for_data_input(ctx)) == (NETWORK_ERROR_NONE))
                    {
                        communication_read_input_stream(ctx->id);
                        trace() << "type: " << st_msg[nodeNumber].msgType.front();
                        st_msg[nodeNumber].msgType.pop();
                    }
                }
                else
                {
                    trace() << "Packet of size 0";
                }

                DataReqMode req_mode = manager_getDataReqMode(nodeNumber);
                //Manager-initiated mode:  Manager request measurement
                if (managerInitiated && ctx->fsm->state == fsm_state_operating)
                {
                    if (communication_agent_getIsTheStartMode(nodeNumber))
                    {
                        if (req_mode & DATA_REQ_SUPP_MODE_SINGLE_RSP)
                        {
                            managerInitiated = false;
                            data_spacing = 0; //data_spacing not allowed in this mode
                            alarmt = 1; //just one measurement
                            dataSN++;
                            setTimer(SEND_PACKET, 0);
                        }
                        else if (req_mode & DATA_REQ_SUPP_MODE_TIME_PERIOD)
                        {
                            managerInitiated = false;
                            alarmt = (int)(managerInitiatedTime * reading_rate);
                            dataSN++;
                            agent_send_data(CONTEXT_ID);
                            setTimer(SEND_PACKET, 0);
                            --alarmt;
                            //An association has been made, update isTheFirstAssociation
                            isTheFirstAssociation = getNumberOfAssociationsTotal(nodeNumber);
                        }
                        else if (req_mode & DATA_REQ_SUPP_MODE_TIME_NO_LIMIT)
                        {
                            managerInitiated = false;
                            alarmt = (int)(maxSimTime * reading_rate);
                            dataSN++;
                            agent_send_data(CONTEXT_ID);
                            setTimer(SEND_PACKET, 0);
                            --alarmt;
                            //An association has been made, update isTheFirstAssociation
                            isTheFirstAssociation = getNumberOfAssociationsTotal(nodeNumber);
                        }
                    }
                    else
                    {
                        dataSN++;
                        setTimer(SEND_PACKET, 0);
                    }
                }
                //Manager-initiated mode: stop sending measurements message received
                else if (ctx->fsm->state == fsm_state_operating && !(req_mode >> 15) && (getNumberOfAssociationsTotal(nodeNumber) == isTheFirstAssociation))
                {
                    alarmt = 0;
                    dataSN++;
                    setTimer(SEND_PACKET, 0);
                }
                //Manager-initiated and Agent-initiated mode: Checks if agent can send measurements
                else if (ctx->fsm->state == fsm_state_operating && alarmt > 0)
                {
                    dataSN++;
                    agent_send_data(CONTEXT_ID);

                    updateTimeOutToRetransmitPacket(SIMTIME_DBL(simTime() - rcvPacket->getCreationTime()));

                    //After an associantion, a packet is sent with no timeout
                    if (getNumberOfAssociationsTotal(nodeNumber) > isTheFirstAssociation)
                    {
                        //The first measurement send after a association
                        setTimer(SEND_PACKET, 0);
                        isTheFirstAssociation = getNumberOfAssociationsTotal(nodeNumber);
                    }
                    else
                        setTimer(SEND_PACKET, data_spacing);

                    --alarmt;
                }
                //Manager-initiated and Agent-initiated mode: request association release
                else if (alarmt == 0 && ctx->fsm->state == fsm_state_operating)
                {
                    agent_request_association_release(CONTEXT_ID);
                    --alarmt;
                    dataSN++;
                    //if (st_msg[nodeNumber].tam_buff > 0)
                    setTimer(SEND_PACKET, 0);
                }
                //Manager-initiated and Agent-initiated mode: association disconnect
                else if (alarmt == -1)
                {
                    //abort message arrived in manager-initiated mode when no
                    //measurements has been sent
                    if (ctx->fsm->state == fsm_state_associating || managerInitiated)
                    { 
                        tryNewAssociationForAbort();
                    }
                    else
                    {
                        agent_disconnect(CONTEXT_ID);
                        --alarmt;
                    }
                }
                //Manager-initiated and Agent-initiated mode:
                else //associantion abort received
                {
                    if ((ctx->fsm->state == fsm_state_unassociated || ctx->fsm->state == fsm_state_associating) && alarmt >= 0)
                    {
                        tryNewAssociationForAbort();
                    }
                }

                context_unlock(ctx);
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
            toNetworkLayer(fwdPacket, "0");
            dataSN++;
        }
    }
}

void Agent::timerFiredCallback(int index)
{
    Context *ctx;
    CONTEXT_ID = {my_plugin_number, 0};
    ctx = context_get_and_lock(CONTEXT_ID);

    switch (index)
    {
    case SEND_PACKET:
    {
        trace() << "Sending packet #" << dataSN << " to node " << recipientAddress.c_str(); //sequence number
        while (!st_msg[nodeNumber].msgType.empty())
        {
            trace() << "type: " << st_msg[nodeNumber].msgType.front();
            st_msg[nodeNumber].msgType.pop();
        }
        toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
        packetsSent[recipientId]++;
        fsm_states c = ctx->fsm->state;

        //Set the timeouts
        switch (c)
        {
        case fsm_state_associating:
        {
            if (RC)
            {
                if (retransmissionPacket){
                    setTimer(TO_ASSOC, 3); //4
                }else{
                    //fprintf(stderr,"\naquiiii\n");
                    setTimer(TO_ASSOC, 10);
                }
            }
            break;
        }
        case fsm_state_operating:
        {
            //Comfirmed event chosen
            if (SETTIMER[nodeNumber])
            {
                //new measurement being transmited
                collectHistogram("Number of transmissions' retries per packet", numOfRetransmissions);
                numOfRetransmissions = 0;
                //3 new associations can be made now
                RC = RC_COUNT;

                if (retransmissionPacket)
                    setTimer(TO_OPERA, timeOutToRetransmitPacket);
                else
                    setTimer(TO_OPERA, 3);

                SETTIMER[nodeNumber] = 0;
            }
            else //Unconfirmed event chosen
            {
                st_msg[nodeNumber].tam_buff = 0;
                st_msg[nodeNumber].buff_msgSed.clear();
                //Manager-initiated mode sends a stop message
                if (managerInitiated)
                {
                    //TO DO
                }
                else if (ctx->fsm->state == fsm_state_operating && alarmt > 0)
                {
                    agent_send_data(CONTEXT_ID);
                    --alarmt;
                    dataSN++;
                    setTimer(SEND_PACKET, data_spacing);
                }
                else if (alarmt == 0)
                {
                    agent_request_association_release(CONTEXT_ID);
                    --alarmt;
                    dataSN++;
                    setTimer(SEND_PACKET, 0);
                }
                else if (alarmt == -1)
                {
                    agent_disconnect(CONTEXT_ID);
                    --alarmt;
                }
            }
            break;
        }
        default:
            break;
        }
        break;
    }

    case TO_ASSOC:
    {
        /**
         * Try 3 association, if manager does not
         * respond, abort.
         * */
        if (RC == 0)
        {
            st_msg[nodeNumber].tam_buff = 0;
            st_msg[nodeNumber].buff_msgSed.clear();
            //trace() << "response not received for packet #" << dataSN << " in association mode aborting...";
            trace() << "3 consecutive associations failed. No longer associations will be made.";
            dataSN++;
            agent_request_association_abort(CONTEXT_ID);
            while (!st_msg[nodeNumber].msgType.empty())
            {
                trace() << "type: " << st_msg[nodeNumber].msgType.front();
                st_msg[nodeNumber].msgType.pop();
            }
            toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
            packetsSent[recipientId]++;
            //RC = RC_COUNT;
            cancelTimer(SEND_PACKET);
            cancelTimer(TO_ASSOC);
            cancelTimer(TO_OPERA);
            agent_disconnect(CONTEXT_ID);
            //agent_finalize(CONTEXT_ID);
        }
        else //Resend the packet (max of 3 times)
        {
            retransmitPacket();
            RC--;

            if (retransmissionPacket)
                setTimer(TO_ASSOC, 3);
            else
                setTimer(TO_ASSOC, 10);
        }
        break;
    }

    case TO_OPERA:
    {
        trace() << "response not received for packet #" << dataSN << " in operating mode";

        /**
         * Retransmission of packets in the
         * application layer is not
         * discussed in 11073 standard,
         * this is an independent modification.
         * */
        //retransmissionMode
        if ((numOfRetransmissions < maxNumOfRetransmition) && retransmissionPacket)
        {
            retransmitPacket();
            numOfRetransmissions++;
            setTimer(TO_OPERA, timeOutToRetransmitPacket);
        }
        //confirmedMode
        else //try a new association
        {
            tryNewAssociationForTimeout();
            //reset the number of retrasmission
            numOfRetransmissions = 0;
        }
        break;
    }
    }
    context_unlock(ctx);
}

/**
 * This method processes a received carrier sense interupt.
 * Used only for demo purposes in some simulations.
 * Feel free to comment out the trace command.
 */
void Agent::handleRadioControlMessage(RadioControlMessage *radioMsg)
{
    switch (radioMsg->getRadioControlMessageKind())
    {
    case CARRIER_SENSE_INTERRUPT:
        trace() << "CS Interrupt received! current RSSI value is: " << radioModule->readRSSI();
        break;
    }
}

void Agent::finishSpecific()
{
    //declareOutput("Packets reception rate");
    //declareOutput("Packets loss rate");
    declareOutput("Control Packets");
    declareOutput("Measurement Packets Sent");
    declareOutput("Total of associations made");

    // temp variable to access packets received by other nodes
    // cTopology *topo;
    // topo = new cTopology("topo");
    // //Extract the module of node to work with.
    // //Extracts model topology by the fully qualified NED type name of the modules.
    // topo->extractByNedTypeName(cStringTokenizer("node.Node").asVector());

    // long bytesDelivered = 0;
    // for (int i = 0; i < numNodes; i++)
    // {
    //     Agent *appModule = dynamic_cast<Agent *>(topo->getNode(i)->getModule()->getSubmodule("Application"));
    //     if (appModule)
    //     {
    //         int packetsSent = appModule->getPacketsSent(self);
    //         if (packetsSent > 0) // this node sent us some packets
    //         {
    //             float rate = (float)packetsReceived[i] / packetsSent;
    //             collectOutput("Packets reception rate", i, "total", rate);
    //             collectOutput("Packets loss rate", i, "total", 1 - rate);
    //         }

    //         bytesDelivered += appModule->getBytesReceived(self);
    //     }
    // }
    collectOutput("Control Packets", HUBNODE, "received", getControlPacketsReceived(nodeNumber));
    collectOutput("Control Packets", HUBNODE, "sent", getControlPacketsSent(nodeNumber));
    collectOutput("Control Packets", HUBNODE, "total", getControlPacketsReceived(nodeNumber) + getControlPacketsSent(nodeNumber));

    collectOutput("Measurement Packets Sent", HUBNODE, "", getMeasurementPacketsTotal(nodeNumber));
    collectOutput("Total of associations made", HUBNODE, "", getNumberOfAssociationsTotal(nodeNumber));

    //delete (topo);
    long bytesDelivered = getBytesReceived(nodeNumber); 
    if (bytesDelivered > 0)
    {
        double energy = (resMgrModule->getSpentEnergy() * 1000000000) / (bytesDelivered * 8); //in nanojoules/bit
        declareOutput("Energy nJ/bit");
        collectOutput("Energy nJ/bit", "", energy);
    }

    if (st_msg != NULL)
    {
        delete[] st_msg;
        st_msg = NULL;
    }

    if (comm_plugin != NULL)
    {
        delete[] comm_plugin;
        comm_plugin = NULL;
    }

    if (SETTIMER != NULL)
    {
        delete[] SETTIMER;
        SETTIMER = NULL;
    }

    CONTEXT_ID = {my_plugin_number, 0};
    agent_finalize(CONTEXT_ID);
}

MyPacket *Agent::createDataPacket(int seqNum)
{
    MyPacket *pkt = new MyPacket("mypacket", APPLICATION_PACKET);
    pkt->setBuffArraySize(st_msg[nodeNumber].tam_buff);
    for (int i = 0; i < st_msg[nodeNumber].tam_buff; i++)
    {
        pkt->setBuff(i, st_msg[nodeNumber].buff_msgSed[i]);
    }
    pkt->setTam_buff(st_msg[nodeNumber].tam_buff);
    pkt->setSequenceNumber(seqNum);
    pkt->setByteLength(pkt->getBuffArraySize() + sizeof(st_msg[nodeNumber].tam_buff));
    //Copy pkt to pktGlobal
    //pktGlobal = pkt->dup();
    // = pkt->getCreationTime()).dbl();
    return pkt;
}

void Agent::tryNewAssociationForTimeout(void)
{
    Context *ctx;
    CONTEXT_ID = {my_plugin_number, 0};
    ctx = context_get_and_lock(CONTEXT_ID);

    st_msg[nodeNumber].tam_buff = 0;
    st_msg[nodeNumber].buff_msgSed.clear();
    while (!st_msg[nodeNumber].msgType.empty())
        st_msg[nodeNumber].msgType.pop();

    trace() << "response not received for packet #" << dataSN << " sending abort message...";
    dataSN++;
    agent_request_association_abort(CONTEXT_ID);
    trace() << "Sending packet #" << dataSN << " to node " << recipientAddress.c_str(); //sequence number
    //trace() << "type: " << st_msg[nodeNumber].msgType.front();
    while (!st_msg[nodeNumber].msgType.empty())
    {
        trace() << "type: " << st_msg[nodeNumber].msgType.front();
        st_msg[nodeNumber].msgType.pop();
    }
    toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
    packetsSent[recipientId]++;

    if (alarmt > 0)
    {
        cancelTimer(SEND_PACKET);
        cancelTimer(TO_ASSOC);
        cancelTimer(TO_OPERA);

        st_msg[nodeNumber].tam_buff = 0;
        st_msg[nodeNumber].buff_msgSed.clear();

        dataSN++;
        trace() << "trying new association";
        service_init(ctx);
        agent_associate(CONTEXT_ID);
        setTimer(SEND_PACKET, 0);
    }

    //reset the RC for the new association
    //RC = RC_COUNT;

    context_unlock(ctx);
}

void Agent::tryNewAssociationForAbort(void)
{
    Context *ctx;
    CONTEXT_ID = {my_plugin_number, 0};
    ctx = context_get_and_lock(CONTEXT_ID);

    // st_msg[nodeNumber].tam_buff = 0;
    // st_msg[nodeNumber].buff_msgSed.clear();

    //trace() << "response not received for packet #" << dataSN << " sending aborting message...";
    //dataSN++;
    //agent_request_association_abort(CONTEXT_ID);
    // while (!st_msg[nodeNumber].msgType.empty())
    // {
    //     trace() << "type: " << st_msg[nodeNumber].msgType.front();
    //     st_msg[nodeNumber].msgType.pop();
    // }
    //toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
    //packetsSent[recipientId]++;

    //if (alarmt > 0)
    // {
    //Checks if there was a measurement to be sent
    if (getTimer(SEND_PACKET) != 0)
        alarmt++;

    cancelTimer(SEND_PACKET);
    cancelTimer(TO_ASSOC);
    cancelTimer(TO_OPERA);
    agent_request_association_abort(CONTEXT_ID);

    st_msg[nodeNumber].tam_buff = 0;
    st_msg[nodeNumber].buff_msgSed.clear();
    while (!st_msg[nodeNumber].msgType.empty())
       st_msg[nodeNumber].msgType.pop();

    service_init(ctx);
    trace() << "trying new association";
    agent_associate(CONTEXT_ID);
    dataSN++;
    setTimer(SEND_PACKET, 0);

    // cancelTimer(SEND_PACKET);
    // cancelTimer(TO_ASSOC);
    // cancelTimer(TO_OPERA);

    // st_msg[nodeNumber].tam_buff = 0;
    // st_msg[nodeNumber].buff_msgSed.clear();

    //dataSN++;

    //service_init(ctx);
    //agent_associate(CONTEXT_ID);
    //setTimer(SEND_PACKET, 0);
    //}

    //reset the RC for the new association
    //RC = RC_COUNT;

    context_unlock(ctx);
}

void Agent::retransmitPacket(void)
{
    trace() << "resending packet #" << dataSN << " to node " << recipientAddress.c_str();
    //toNetworkLayer(pktGlobal, recipientAddress.c_str());
    toNetworkLayer(createDataPacket(dataSN), recipientAddress.c_str());
    packetsSent[recipientId]++;
    //collectOutput("Number of transmissions retries per packet", dataSN);
}

void Agent::updateTimeOutToRetransmitPacket(double t){
    timeOutToRetransmitPacket = t * 1000;
}
