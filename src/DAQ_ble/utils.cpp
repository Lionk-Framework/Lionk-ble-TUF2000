#include <WiFi.h>
#include <stdint.h>

#include <array>

#include "utils.h"

uint64_t getDeviceId()
{
	uint64_t id = 0;
	std::array<uint8_t, 6> mac;
	WiFi.macAddress(mac.data());
	for (const auto &byte : mac) {
		id = (id << 8) | byte;
	}
	return id;
}
