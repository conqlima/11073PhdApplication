cplusplus {{
#include "ApplicationPacket_m.h"
}}

class ApplicationPacket;

packet MyPacket extends ApplicationPacket {
	uint8_t buff[];
	int tam_buff;
}

