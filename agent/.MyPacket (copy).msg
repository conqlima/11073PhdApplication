cplusplus {{
#include "ApplicationPacket_m.h"

extern "C" {
#include "communication/plugin/plugin.h"
#include "util/bytelib.h"
}

typedef ByteStreamReader *ByteStreamReaderPtr;
typedef ByteStreamWriter *ByteStreamWriterPtr;

}}

struct ByteStreamReaderPtr;
struct ByteStreamWriterPtr;
class ApplicationPacket;

struct Tmsg {
	ByteStreamReaderPtr byter;
	ByteStreamWriterPtr bytew;
}

packet MyPacket extends ApplicationPacket {
	Tmsg extraData;
}

