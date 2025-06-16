#include "json_parser.h"
#include "../models/sensor_data.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<SensorData> JsonParser::parseSensorDataArray(const std::string& body) {
    std::vector<SensorData> result;

    try {
        auto json_array = json::parse(body);
        if (!json_array.is_array()) {
            throw std::runtime_error("Expected JSON array");
        }

        for (const auto& item : json_array) {
            SensorData data;
            data.sensor_id = item.at("sensor_id").get<std::string>();
            data.value = item.at("value").get<double>();

            // timestamp: как UNIX-время (int64_t), преобразуем в time_point
            std::time_t timestamp_sec = item.at("timestamp").get<std::time_t>();
            data.timestamp = std::chrono::system_clock::from_time_t(timestamp_sec);

            result.push_back(std::move(data));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Failed to parse JSON: ") + e.what());
    }

    return result;
}
