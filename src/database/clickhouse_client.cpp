#include "clickhouse_client.h"

using namespace clickhouse;

ClickHouseClient::ClickHouseClient(const std::string& host, int port, const std::string& database) {
    ClientOptions options;
    options.SetHost(host);
    options.SetPort(port);
    options.SetDefaultDatabase(database);
    client_ = std::make_unique<Client>(options);
}

bool ClickHouseClient::insertBatch(const std::vector<SensorData>& batch) {
    try {
        auto block = std::make_shared<Block>();

        auto col_sensor_id = std::make_shared<ColumnString>();
        auto col_value = std::make_shared<ColumnFloat64>();
        auto col_timestamp = std::make_shared<ColumnUInt64>();

        for (const auto& data : batch) {
            col_sensor_id->Append(data.sensor_id);
            col_value->Append(data.value);
            auto ts = std::chrono::duration_cast<std::chrono::seconds>(
                data.timestamp.time_since_epoch()).count();
            col_timestamp->Append(static_cast<uint64_t>(ts));
        }

        block->AppendColumn("sensor_id", col_sensor_id);
        block->AppendColumn("value", col_value);
        block->AppendColumn("timestamp", col_timestamp);

        client_->Insert("sensor_data", *block);

        return true;
    } catch (const std::exception& ex) {
        return false;
    }
}
