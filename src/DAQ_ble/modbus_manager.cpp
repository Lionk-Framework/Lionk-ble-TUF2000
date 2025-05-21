#include <ModbusMaster.h>

#include "utils.h"

#define RS485_DE_RE_PIN	    2
#define MODBUS_SLAVE_ID	    1
#define MODBUS_BAUDRATE	    9600

#define FLOW_REGISTER_ADDR  0x0001
#define FLOW_REGISTER_COUNT 2

ModbusMaster node;

void preTransmission()
{
	digitalWrite(RS485_DE_RE_PIN, HIGH);
}

void postTransmission()
{
	digitalWrite(RS485_DE_RE_PIN, LOW);
}

void modbusSetup()
{
	pinMode(RS485_DE_RE_PIN, OUTPUT);
	digitalWrite(RS485_DE_RE_PIN, LOW); // Set RS485 to receive mode

	Serial1.begin(MODBUS_BAUDRATE);
	node.begin(MODBUS_SLAVE_ID, Serial1);
	node.preTransmission(preTransmission);
	node.postTransmission(postTransmission);
}

bool readFlowValue(float &flow)
{
	uint8_t result = node.readHoldingRegisters(FLOW_REGISTER_ADDR,
						   FLOW_REGISTER_COUNT);

	if (result == node.ku8MBSuccess) {
		// According to the documentation, the format is REAL4 (IEEE-754 float)
		// Note: Some devices use swapped word order, check your sensor's output if
		// values are wrong
		uint32_t raw = ((uint32_t)node.getResponseBuffer(0) << 16) |
			       node.getResponseBuffer(1);
		memcpy(&flow, &raw, sizeof(float));
		return true;
	}

	DEBUG_PRINT("Modbus error: ");
	DEBUG_PRINTLN(result);
	return false;
}
