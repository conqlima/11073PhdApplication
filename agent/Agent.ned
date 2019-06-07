package node.application.agent;
simple Agent like node.application.iApplication
	{
	parameters:
		string applicationID = default(""); //mandatory parameter
		bool collectTraceInfo = default(false); // mandatory parameter
		int priority = default(1); // mandatory parameter
		int packetHeaderOverhead = default(16);	// in bytes
		int constantDataPayload = default(16);	// in bytes
		
		string application_type = default("pulseoximeter"); // agent types
		double measurements_per_second = default(4); // measurement per second
		bool confirmed_event = default(false);
		int maxNumOfRetransmition = default(3); // max Number of retransmission of
												// measurement packets
		double timeOutToRetransmitPacket = default(3);
		int hubNode = default (0);		// explict the hub (sink node) node number
		bool retransmitPacket = default(false);
		bool managerInitiated = default(false);
		string managerInitiateMode @enum("singleMode", "timePeriodMode", "noTimePeriodMode") = default("singleMode");
		double managerInitiatedTime = default(0);
		double numberOfReceivedMeasurementsToSendStop = default(0);
		
		//copied from ThroughputTest.ned
		
		double delayLimit = default (0); 		// application delay limit (0 - no limit)

		string nextRecipient = default ("0");	// Destination for packets received in this node. 
												// These parameters can be used to create an
												// application-level static routing. This way we can
												// have a multi-hop throughput test   

		double packet_rate = default (0);		// packets per second, by default we transmit no packets
		double startupDelay = default (0);		// delay in seconds before the app stars producing packets

		double latencyHistogramMax = default (200);
		int latencyHistogramBuckets = default (10);


	gates: 
	 	output toCommunicationModule;
		output toSensorDeviceManager;
		input fromCommunicationModule;
		input fromSensorDeviceManager;
		input fromResourceManager;
	}
