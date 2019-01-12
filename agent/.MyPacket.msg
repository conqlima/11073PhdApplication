cplusplus {{
#include "ApplicationPacket_m.h"

extern "C" {
#include "communication/plugin/plugin.h"
#include "util/bytelib.h"
}

}}

class ApplicationPacket;

struct Tmsg {
	string send_str;
	string recv_str;
}

packet MyPacket extends ApplicationPacket {
	Tmsg extraData;
}

