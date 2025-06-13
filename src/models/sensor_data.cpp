#pragma once
#include <string>
#include <chrono>

struct SensorData {
    std::string sensor_id;
    double value;
    std::chrono::system_clock::time_point timestamp;
};
