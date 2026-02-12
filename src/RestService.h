#pragma once
#include <rest/Service.h>
#include <solax/Telemetry.h>
#include <atomic>
#include <vector>

namespace solax
{

class RestService
{
public:
    struct Config
    {
        std::string address{};
        uint16_t port{};
    };

    static Config loadConfig(const std::string& configPath);

    RestService(const Config& config);
    ~RestService();

    void updateTelemetry(const solax::AggregatedTelemetry& newAggregatedTelemetry,
                         const std::vector<solax::UnitTelemetry>& newUnitTelemetries);

private:
    std::unique_ptr<rest::Service> service;

    std::atomic_int latestTelemetryIndex{0};
    std::array<solax::AggregatedTelemetry, 2> latestAggregatedTelemetry;
    std::array<std::vector<solax::UnitTelemetry>, 2> latestUnitTelemetries;

    void handleRequest(const std::vector<utility::string_t>& paths, web::http::http_request& message);
};



}