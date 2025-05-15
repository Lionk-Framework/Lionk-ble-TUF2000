#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

#define PAYLOAD_VERSION 0
#define HEADER_SIZE	2
#define SAMPLES_SIZE	2
#define SAMPLES_NUMBER	2
#define PAYLOAD_SIZE	(HEADER_SIZE + SAMPLES_NUMBER)

typedef struct {
	uint16_t flow_rate;
} sensor_data_t;

extern sensor_data_t sensor_data;

void updateSensorData();
void buildSensorDataBuffer(const sensor_data_t *data, uint8_t *buf);

#endif // SENSOR_H
