#include "ble_manager.h"
#include "sensor.h"
#include "utils.h"
#include "modbus_manager.h"

sensor_data_t sensor_data;
static int i = 0;

void updateSensorData()
{
	float flowValue = 0.0f;
	if (readFlowValue(flowValue)) {
		sensor_data.flow_rate = static_cast<uint16_t>(flowValue * 100);
		DEBUG_PRINT("Flow rate (Modbus): ");
		DEBUG_PRINT(sensor_data.flow_rate);
		DEBUG_PRINTLN(" (raw value)");
		DEBUG_PRINT("Actual flow: ");
		DEBUG_PRINT(flowValue);
		DEBUG_PRINTLN(" L/min");
	} else {
		DEBUG_PRINTLN("Failed to read flow from Modbus");
		sensor_data.flow_rate = 0;
	}
	flowChar.writeValue(sensor_data.flow_rate);
}

void buildSensorDataBuffer(const sensor_data_t *data, uint8_t *buf)
{
	buf[0] = PAYLOAD_VERSION;
	buf[1] = SAMPLES_PER_PACKET;
	buf[2] = static_cast<uint8_t>((data->flow_rate & 0xFF00) >> 8);
	buf[3] = static_cast<uint8_t>(data->flow_rate & 0xFF);
}
