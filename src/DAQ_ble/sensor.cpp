#include "ble_manager.h"
#include "sensor.h"
#include "utils.h"
#include "modbus_manager.h"

sensor_data_t sensor_data;
static int i = 0;

void updateSensorData()
{   
	// Read flow rate
	float flowValue = 0.0f;
	if (flowHistorySubscribed && readFlowValue(flowValue)) {
		sensor_data.flow_rate = static_cast<uint16_t>(flowValue * 100);
		DEBUG_PRINT("Flow rate (Modbus): ");
		DEBUG_PRINT(sensor_data.flow_rate);
		DEBUG_PRINTLN(" (raw value)");
		DEBUG_PRINT("Actual flow: ");
		DEBUG_PRINT(flowValue);
		DEBUG_PRINTLN(" L/min");
	} else {
		DEBUG_PRINTLN("Failed to read flow from Modbus");
	}
	
	// Read velocity
	float velocityValue = 0.0f;
	if (velocityHistorySubscribed && readVelocityValue(velocityValue)) {
		sensor_data.velocity = static_cast<uint16_t>(velocityValue * 100);
		DEBUG_PRINT("Velocity (Modbus): ");
		DEBUG_PRINT(sensor_data.velocity);
		DEBUG_PRINTLN(" (raw value)");
		DEBUG_PRINT("Actual velocity: ");
		DEBUG_PRINT(velocityValue);
		DEBUG_PRINTLN(" m/s");
	} else {
		DEBUG_PRINTLN("Failed to read velocity from Modbus");
		sensor_data.velocity = 0;
	}
	
	// Read pipe diameter
	float diameterValue = 0.0f;
    float oldDiameterValue = sensor_data.pipe_diameter;
	if (pipeDiameterSubscribed && readInnerPipeDiameter(diameterValue)) {
		sensor_data.pipe_diameter = static_cast<uint16_t>(diameterValue * 100);
        if(sensor_data.pipe_diameter != 0 && oldDiameterValue != diameterValue){
	        pipeDiameterChar.writeValue(sensor_data.pipe_diameter);
        }
        
		DEBUG_PRINT("Pipe diameter (Modbus): ");
		DEBUG_PRINT(sensor_data.pipe_diameter);
		DEBUG_PRINTLN(" (raw value)");
		DEBUG_PRINT("Actual diameter: ");
		DEBUG_PRINT(diameterValue);
		DEBUG_PRINTLN(" mm");
	} else {
		DEBUG_PRINTLN("Failed to read pipe diameter from Modbus");
		sensor_data.pipe_diameter = 0;
	}
}

void updateYearlyFlowData()
{
	// Read yearly flow
	uint32_t yearlyFlowValue = 0;

	if (yearlyFlowSubscribed && readYearlyFlow(yearlyFlowValue)) {
		sensor_data.yearly_flow = yearlyFlowValue;
		DEBUG_PRINT("Yearly Flow (Modbus): ");
		DEBUG_PRINT(sensor_data.yearly_flow);
		DEBUG_PRINTLN(" units");
	} else {
		DEBUG_PRINTLN("Failed to read yearly flow from Modbus");
		sensor_data.yearly_flow = 0;
	}
	yearlyFlowChar.writeValue(sensor_data.yearly_flow);
}

void buildSensorDataBuffer(const sensor_data_t *data, uint8_t *buf)
{
	// Set header info
	buf[0] = PAYLOAD_VERSION;
	buf[1] = SAMPLES_PER_PACKET;
	
	// Only writing the current flow value - this function is not currently used 
	// as we're primarily using bufferSend methods instead of single value updates
	buf[2] = static_cast<uint8_t>((data->flow_rate & 0xFF00) >> 8);
	buf[3] = static_cast<uint8_t>(data->flow_rate & 0xFF);
}
