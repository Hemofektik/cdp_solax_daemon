#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include <thread>
#include "backward.hpp"

#include <solax/SerialAdapter.h>
#include <solax/Telemetry.h>
#include "RestService.h"
#include "Config.h"

using namespace std::chrono_literals;
using namespace solax;

auto to_string(int32_t x) -> std::string { return std::to_string(x); };

std::ostream& operator<<(std::ostream& os, const std::optional<int32_t>& opt)
{
    os << opt.transform( to_string ).value_or("-");
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::optional<std::string>& opt)
{
    os << opt.value_or("-");
    return os;
}

int main()
{
    backward::SignalHandling sh;

    const auto config{loadConfig("solax.cfg")};

    std::optional<RestService> restService;

    try
    {
        restService.emplace(config.rest);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Unable to create REST service: "  << e.what() << std::endl;
        return 1;
    }

    const bool debugLogEnabled{false};
    std::optional<SerialAdapter> serialAdapter{};
    std::vector<UnitTelemetry> unitTelemetries;
    unitTelemetries.reserve(8);
    
    while(true)
    {
        try
        {
            if(!serialAdapter)
            {
                std::cout << "Connecting to Solax serial adapter" << std::endl;
                serialAdapter.emplace(config.serialAdapter);
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            std::this_thread::sleep_for(10s);
            continue;
        }

        std::this_thread::sleep_for(100ms);

        try
        {
            unitTelemetries.clear();
            uint8_t machineIndex = 1;
            while(machineIndex > 0)
            {
                auto const rawTelemetry{serialAdapter->readRawTelemetry(machineIndex)};
                auto const unitTelemetry{parseRawTelemetry(rawTelemetry)};
                
                if(unitTelemetry.parallelNum == 0)
                {
                    // No more machines available
                    break;
                }

                machineIndex++;
                unitTelemetries.push_back(unitTelemetry);
            }

            auto const aggregatedTelemetry{aggregateTelemetry(unitTelemetries)};

            if(debugLogEnabled)
            {
                std::cout << "Machines: " << unitTelemetries.size() << ", "
                          << "Solar: " << aggregatedTelemetry.solarPower_W << " W, "
                          << "AC: " << aggregatedTelemetry.acPower_W << " W, "
                          << "Battery: " << aggregatedTelemetry.batteryPower_W << " W"
                          << std::endl;
            }

            restService->updateTelemetry(aggregatedTelemetry, unitTelemetries);

        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            serialAdapter.reset();
        }
    }
}
