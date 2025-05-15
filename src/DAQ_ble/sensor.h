#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

// --- Payload and BLE packet configuration ---
#define PAYLOAD_VERSION	   0
#define HEADER_SIZE	   2
#define SAMPLE_SIZE	   2
#define SAMPLES_PER_PACKET 10
#define PAYLOAD_SIZE	   (HEADER_SIZE + SAMPLES_PER_PACKET * SAMPLE_SIZE)
// -------------------------------------------

typedef struct {
	uint16_t flow_rate;
} sensor_data_t;

extern sensor_data_t sensor_data;

void updateSensorData();
void buildSensorDataBuffer(const sensor_data_t *data, uint8_t *buf);

#endif // SENSOR_H
