#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>
#include "modbus_manager.h"

// --- Payload and BLE packet configuration ---
#define PAYLOAD_VERSION	   0
#define HEADER_SIZE	   2
#define SAMPLE_SIZE	   2
#define SAMPLES_PER_PACKET 50
#define PAYLOAD_SIZE	   (HEADER_SIZE + SAMPLES_PER_PACKET * SAMPLE_SIZE)

// Update periods
#define FLOW_VELOCITY_UPDATE_INTERVAL 20    // 20ms (50 samples/second)
#define YEARLY_FLOW_UPDATE_INTERVAL   10000 // 10 seconds
// -------------------------------------------

typedef struct {
	uint16_t flow_rate;      // Flow rate (scaled by 100)
	uint16_t velocity;       // Velocity (scaled by 100)
	uint16_t pipe_diameter;  // Pipe diameter (scaled by 100)
	uint32_t yearly_flow;    // Flow for this year
} sensor_data_t;

extern sensor_data_t sensor_data;

void updateSensorData();
void updateYearlyFlowData();
void buildSensorDataBuffer(const sensor_data_t *data, uint8_t *buf);

#endif // SENSOR_H
