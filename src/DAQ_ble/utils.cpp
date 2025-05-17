#include <WiFi.h>
#include <stdint.h>
#include <string.h>

#include <array>

#include "utils.h"

String getDeviceId()
{
	uint64_t id = 0;
	std::array<uint8_t, 6> mac;
	WiFi.macAddress(mac.data());

        String mac_str;
        for (size_t i = 0; i < mac.size(); ++i) {
            if (i != 0) {
                mac_str += ":";
            }
            mac_str += String(mac[i], HEX);
        }

        DEBUG_PRINT("MAC Address: ");
        return mac_str;
}
