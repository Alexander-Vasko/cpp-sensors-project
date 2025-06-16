#pragma once

#include <string>
#include <vector>
#include "../models/sensor_data.h"

class JsonParser {
public:
    static std::vector<SensorData> parseSensorDataArray(const std::string& body);
};
