#include <ModbusMaster.h>

#include "utils.h"

#define RS485_DE_RE_PIN	    2
#define MODBUS_SLAVE_ID	    1
#define MODBUS_BAUDRATE	    9600

// Registre Modbus
#define FLOW_REGISTER_ADDR         0x0001
#define FLOW_REGISTER_COUNT        2
#define VELOCITY_REGISTER_ADDR     0x0005
#define VELOCITY_REGISTER_COUNT    2
#define PIPE_DIAM_REGISTER_ADDR    0x00DD  // 221
#define PIPE_DIAM_REGISTER_COUNT   2
#define YEAR_FLOW_REGISTER_ADDR    0x0091  // 145
#define YEAR_FLOW_REGISTER_COUNT   2

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

	DEBUG_PRINT("Modbus error reading flow: ");
	DEBUG_PRINTLN(result);
	return false;
}

bool readVelocityValue(float &velocity)
{
	uint8_t result = node.readHoldingRegisters(VELOCITY_REGISTER_ADDR,
						   VELOCITY_REGISTER_COUNT);

	if (result == node.ku8MBSuccess) {
		uint32_t raw = ((uint32_t)node.getResponseBuffer(0) << 16) |
			       node.getResponseBuffer(1);
		memcpy(&velocity, &raw, sizeof(float));
		return true;
	}

	DEBUG_PRINT("Modbus error reading velocity: ");
	DEBUG_PRINTLN(result);
	return false;
}

bool readInnerPipeDiameter(float &diameter)
{
	uint8_t result = node.readHoldingRegisters(PIPE_DIAM_REGISTER_ADDR,
						   PIPE_DIAM_REGISTER_COUNT);

	if (result == node.ku8MBSuccess) {
		uint32_t raw = ((uint32_t)node.getResponseBuffer(0) << 16) |
			       node.getResponseBuffer(1);
		memcpy(&diameter, &raw, sizeof(float));
		return true;
	}

	DEBUG_PRINT("Modbus error reading pipe diameter: ");
	DEBUG_PRINTLN(result);
	return false;
}

bool writeInnerPipeDiameter(float diameter)
{
	uint32_t raw;
	memcpy(&raw, &diameter, sizeof(float));
	
	uint16_t highWord = (raw >> 16) & 0xFFFF;
	uint16_t lowWord = raw & 0xFFFF;
	
	uint8_t result = node.writeSingleRegister(PIPE_DIAM_REGISTER_ADDR, highWord);
	if (result != node.ku8MBSuccess) {
		DEBUG_PRINT("Modbus error writing pipe diameter high word: ");
		DEBUG_PRINTLN(result);
		return false;
	}
	
	result = node.writeSingleRegister(PIPE_DIAM_REGISTER_ADDR + 1, lowWord);
	if (result != node.ku8MBSuccess) {
		DEBUG_PRINT("Modbus error writing pipe diameter low word: ");
		DEBUG_PRINTLN(result);
		return false;
	}
	
	return true;
}

bool readYearlyFlow(uint32_t &yearlyFlow)
{
	uint8_t result = node.readHoldingRegisters(YEAR_FLOW_REGISTER_ADDR,
						   YEAR_FLOW_REGISTER_COUNT);

	if (result == node.ku8MBSuccess) {
		// Format LONG (32-bit integer)
		yearlyFlow = ((uint32_t)node.getResponseBuffer(0) << 16) |
			      node.getResponseBuffer(1);
		return true;
	}

	DEBUG_PRINT("Modbus error reading yearly flow: ");
	DEBUG_PRINTLN(result);
	return false;
}
