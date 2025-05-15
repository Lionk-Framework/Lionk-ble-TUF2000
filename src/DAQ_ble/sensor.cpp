#include "ble_manager.h"
#include "sensor.h"
#include "utils.h"

sensor_data_t sensor_data;
static int i = 0;

void updateSensorData()
{
	// TODO: Replace with actual sensor reading
	double flowValue = 10 + ((i++) % 13);
	uint16_t scaledFlow = static_cast<uint16_t>(flowValue * 100);
	sensor_data.flow_rate = scaledFlow;
	flowChar.writeValue(sensor_data.flow_rate);
	DEBUG_PRINT("Flow rate: ");
	DEBUG_PRINT(sensor_data.flow_rate);
	DEBUG_PRINTLN(" (raw value)");
	DEBUG_PRINT("Actual flow: ");
	DEBUG_PRINT(flowValue);
	DEBUG_PRINTLN(" L/min");
}

void buildSensorDataBuffer(const sensor_data_t *data, uint8_t *buf)
{
	buf[0] = PAYLOAD_VERSION;
	buf[1] = SAMPLES_NUMBER;
	buf[2] = static_cast<uint8_t>((data->flow_rate & 0xFF00) >> 8);
	buf[3] = static_cast<uint8_t>(data->flow_rate & 0xFF);
}
