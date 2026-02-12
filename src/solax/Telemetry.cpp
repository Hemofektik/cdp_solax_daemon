#include <solax/Telemetry.h>
#include <CppLinuxSerial/SerialPort.hpp>
#include "OptionalValue.h"
#include <sstream>
#include <thread>
#include <chrono>
#include <ranges>
#include <map>
#include <numeric>
#include <algorithm>

using namespace std::chrono_literals;
using namespace mn::CppLinuxSerial;

namespace solax
{

UnitTelemetry parseRawTelemetry(const std::string& rawTelemetry)
{
    UnitTelemetry ut;
    
    // Strip the 2-byte CRC from the end of the response
    std::string data = rawTelemetry;
    if (data.size() >= 2) {
        data = data.substr(0, data.size() - 2);
    }
    
    std::istringstream iss{data};

    // Parse all fields according to QPGSn protocol
    iss >> ut.parallelNum;                      // A: Parallel num
    iss >> ut.serialNumber;                     // B: Serial number
    iss >> ut.workMode;                         // C: Work mode
    iss >> ut.faultCode;                        // D: Fault code
    iss >> ut.gridVoltage_V;                    // E: Grid voltage
    iss >> ut.gridFrequency_Hz;                 // F: Grid frequency
    iss >> ut.acOutputVoltage_V;                // G: AC output voltage
    iss >> ut.acOutputFrequency_Hz;             // H: AC output frequency
    iss >> ut.acOutputApparentPower_VA;         // I: AC output apparent power
    iss >> ut.acOutputActivePower_W;            // J: AC output active power
    iss >> ut.loadPercent;                      // K: Load percentage
    iss >> ut.batteryVoltage_V;                 // L: Battery voltage
    iss >> ut.batteryChargingCurrent_A;         // M: Battery charging current
    iss >> ut.batteryCapacity_pct;              // N: Battery capacity
    iss >> ut.pv1InputVoltage_V;                // O: PV1 input voltage
    iss >> ut.totalChargingCurrent_A;           // P: Total charging current
    iss >> ut.totalAcOutputApparentPower_VA;    // Q: Total AC output apparent power
    iss >> ut.totalOutputActivePower_W;         // R: Total output active power
    iss >> ut.totalAcOutputPercent;             // S: Total AC output percentage
    iss >> ut.inverterStatus;                   // U: Inverter status (8 bits)
    iss >> ut.outputMode;                       // T: Output mode
    iss >> ut.chargerSourcePriority;            // U: Charger source priority
    iss >> ut.maxChargerCurrent_A;              // V: Max charger current
    iss >> ut.maxChargerRange_A;                // W: Max charger range
    iss >> ut.maxAcChargerCurrent_A;            // Z: Max AC charger current
    iss >> ut.pv1InputCurrent_A;                // a: PV1 input current
    iss >> ut.batteryDischargeCurrent_A;        // b: Battery discharge current
    iss >> ut.pv2InputVoltage_V;                // c: PV2 input voltage
    iss >> ut.pv2InputCurrent_A;                // d: PV2 input current

    return ut;
}

AggregatedTelemetry aggregateTelemetry(const std::vector<UnitTelemetry>& unitTelemetry)
{
    AggregatedTelemetry agg{0.0f, 0.0f, 0.0f};
    
    for (const auto& ut : unitTelemetry) {
        // Solar power: PV1 + PV2 (voltage * current)
        float pv1Power = ut.pv1InputVoltage_V * static_cast<float>(ut.pv1InputCurrent_A);
        float pv2Power = ut.pv2InputVoltage_V * static_cast<float>(ut.pv2InputCurrent_A);
        agg.solarPower_W += pv1Power + pv2Power;
        
        // AC output active power
        agg.acPower_W += static_cast<float>(ut.acOutputActivePower_W);
        
        // Battery power: charging current - discharge current (positive = charging)
        float batteryPower = ut.batteryVoltage_V * 
            (static_cast<float>(ut.batteryChargingCurrent_A) - static_cast<float>(ut.batteryDischargeCurrent_A));
        agg.batteryPower_W += batteryPower;
    }
    
    return agg;
}


}