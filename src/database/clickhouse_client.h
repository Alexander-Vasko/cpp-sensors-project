#pragma once
#include <clickhouse/client.h>
#include <string>
#include <memory>
#include <vector>
#include "../models/sensor_data.h"

class ClickHouseClient {
public:
    ClickHouseClient(const std::string& host, int port, const std::string& database);

    bool insertBatch(const std::vector<SensorData>& batch);

private:
    std::unique_ptr<clickhouse::Client> client_;
};
