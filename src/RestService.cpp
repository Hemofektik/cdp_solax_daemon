#include "RestService.h"
#include "cpprest/uri.h"

namespace solax
{

using namespace web::http;

namespace {

utility::string_t BasePath{U("telemetry")};
utility::string_t SolarPower{U("solarPower_W")};
utility::string_t AcPower{U("acPower_W")};
utility::string_t BatteryPower{U("batteryPower_W")};

auto asJson(const solax::AggregatedTelemetry& telemetry)
{
    web::json::value res = web::json::value::object();
    res[SolarPower] = web::json::value::number(telemetry.solarPower_W);
    res[AcPower] = web::json::value::number(telemetry.acPower_W);
    res[BatteryPower] = web::json::value::number(telemetry.batteryPower_W);
    return res;
}

auto asJson(const solax::UnitTelemetry& unit)
{
    web::json::value res = web::json::value::object();
    res[U("parallelNum")] = web::json::value::number(unit.parallelNum);
    res[U("serialNumber")] = web::json::value::string(unit.serialNumber);
    res[U("workMode")] = web::json::value::string(utility::string_t(1, unit.workMode));
    res[U("faultCode")] = web::json::value::number(unit.faultCode);
    res[U("gridVoltage_V")] = web::json::value::number(unit.gridVoltage_V);
    res[U("gridFrequency_Hz")] = web::json::value::number(unit.gridFrequency_Hz);
    res[U("acOutputVoltage_V")] = web::json::value::number(unit.acOutputVoltage_V);
    res[U("acOutputFrequency_Hz")] = web::json::value::number(unit.acOutputFrequency_Hz);
    res[U("acOutputApparentPower_VA")] = web::json::value::number(unit.acOutputApparentPower_VA);
    res[U("acOutputActivePower_W")] = web::json::value::number(unit.acOutputActivePower_W);
    res[U("loadPercent")] = web::json::value::number(unit.loadPercent);
    res[U("batteryVoltage_V")] = web::json::value::number(unit.batteryVoltage_V);
    res[U("batteryChargingCurrent_A")] = web::json::value::number(unit.batteryChargingCurrent_A);
    res[U("batteryCapacity_pct")] = web::json::value::number(unit.batteryCapacity_pct);
    res[U("pv1InputVoltage_V")] = web::json::value::number(unit.pv1InputVoltage_V);
    res[U("totalChargingCurrent_A")] = web::json::value::number(unit.totalChargingCurrent_A);
    res[U("totalAcOutputApparentPower_VA")] = web::json::value::number(unit.totalAcOutputApparentPower_VA);
    res[U("totalOutputActivePower_W")] = web::json::value::number(unit.totalOutputActivePower_W);
    res[U("totalAcOutputPercent")] = web::json::value::number(unit.totalAcOutputPercent);
    res[U("outputMode")] = web::json::value::number(unit.outputMode);
    res[U("chargerSourcePriority")] = web::json::value::number(unit.chargerSourcePriority);
    res[U("maxChargerCurrent_A")] = web::json::value::number(unit.maxChargerCurrent_A);
    res[U("maxChargerRange_A")] = web::json::value::number(unit.maxChargerRange_A);
    res[U("maxAcChargerCurrent_A")] = web::json::value::number(unit.maxAcChargerCurrent_A);
    res[U("pv1InputCurrent_A")] = web::json::value::number(unit.pv1InputCurrent_A);
    res[U("batteryDischargeCurrent_A")] = web::json::value::number(unit.batteryDischargeCurrent_A);
    res[U("pv2InputVoltage_V")] = web::json::value::number(unit.pv2InputVoltage_V);
    res[U("pv2InputCurrent_A")] = web::json::value::number(unit.pv2InputCurrent_A);
    return res;
}

auto asJson(const char* message)
{
    web::json::value res = web::json::value::object();
    res[U("error")] = web::json::value::string(message);
    return res;
}

}

RestService::RestService(const Config& config)
{   
    try
    {
        utility::string_t port{std::to_string(config.port)};
        utility::string_t address{U("http://")};
        address.append(config.address);
        address.append(U(":"));
        address.append(port);

        web::uri_builder uri(address);
        uri.append_path(BasePath);

        utility::string_t fullUriStr{uri.to_uri().to_string()};       
        std::cout << "Listening for requests at: " << fullUriStr << std::endl;

        service = std::make_unique<rest::Service>(fullUriStr, std::bind(&RestService::handleRequest, this, std::placeholders::_1, std::placeholders::_2));
        service->open().wait();
    }
    catch(const boost::wrapexcept<boost::system::system_error>& e)
    {
        throw std::runtime_error(e.what());
    }
}

RestService::~RestService()
{
    service->close().wait();
}

void RestService::updateTelemetry(const solax::AggregatedTelemetry& newAggregatedTelemetry,
                                  const std::vector<solax::UnitTelemetry>& newUnitTelemetries)
{
    const int newLatestTelemetryIndex{(latestTelemetryIndex + 1) & 1};
    latestAggregatedTelemetry[newLatestTelemetryIndex] = newAggregatedTelemetry;
    latestUnitTelemetries[newLatestTelemetryIndex] = newUnitTelemetries;
    latestTelemetryIndex = newLatestTelemetryIndex;
}

void RestService::handleRequest(const std::vector<utility::string_t>& paths, http_request& message)
{
    try
    {
        if(paths[0] == "aggregated")
        {
            message.reply(status_codes::OK, asJson(latestAggregatedTelemetry[latestTelemetryIndex]));
            return;
        }
        
        const auto& basePath{paths[0]};
        const int machineNumber{std::stoi(basePath)};

        const auto& unitTelemetries{latestUnitTelemetries[latestTelemetryIndex]};

        if(machineNumber < 1 || machineNumber > static_cast<int>(unitTelemetries.size()))
        {
            throw std::out_of_range("Machine number must be between 1 and " + std::to_string(unitTelemetries.size()));
        }

        message.reply(status_codes::OK, asJson(unitTelemetries[machineNumber - 1]));
    }
    catch (std::invalid_argument const& ex)
    {
        message.reply(status_codes::BadRequest, asJson(ex.what()));
    }
    catch (std::out_of_range const& ex)
    {
        message.reply(status_codes::BadRequest, asJson(ex.what()));
    }
}

}