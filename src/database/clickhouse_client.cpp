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

        block->AppendColumn("sensor_id", std::make_shared<ColumnString>());
        block->AppendColumn("value", std::make_shared<ColumnFloat64>());
        block->AppendColumn("timestamp", std::make_shared<ColumnUInt64>());

        for (const auto& data : batch) {
            block->GetColumnByName("sensor_id")->As<ColumnString>()->Append(data.sensor_id);
            block->GetColumnByName("value")->As<ColumnFloat64>()->Append(data.value);

            // преобразуем std::chrono::system_clock::time_point в UNIX timestamp uint64_t
            auto ts = std::chrono::duration_cast<std::chrono::seconds>(data.timestamp.time_since_epoch()).count();
            block->GetColumnByName("timestamp")->As<ColumnUInt64>()->Append(static_cast<uint64_t>(ts));
        }

        client_->Insert("sensor_data", *block);

        return true;
    }
    catch (const std::exception& ex) {
        // Логируем ошибку или обрабатываем иначе
        return false;
    }
}
