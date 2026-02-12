
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <tuple>

#include <solax/Telemetry.h>

using namespace solax;
using Catch::Matchers::WithinAbs;

std::tuple<std::vector<UnitTelemetry>, AggregatedTelemetry> testCase(
    const std::vector<UnitTelemetry>& inputState, 
    const AggregatedTelemetry& expectedOutput)
{
    return std::tuple<std::vector<UnitTelemetry>, AggregatedTelemetry>(inputState, expectedOutput);
}

SCENARIO( "Solax Telemetry can be aggregated", "[solax::telemetry]" ) 
{
    SECTION("Aggregate empty telemetry")
    {
        const std::vector<UnitTelemetry> emptyTelemetry{};
        auto aggregated = aggregateTelemetry(emptyTelemetry);
        
        CHECK_THAT(aggregated.solarPower_W, WithinAbs(0.0, 0.01));
        CHECK_THAT(aggregated.acPower_W, WithinAbs(0.0, 0.01));
        CHECK_THAT(aggregated.batteryPower_W, WithinAbs(0.0, 0.01));
    }
    
    SECTION("Aggregate single unit telemetry")
    {
        UnitTelemetry ut;
        ut.pv1InputVoltage_V = 138.9f;
        ut.pv1InputCurrent_A = 6;
        ut.pv2InputVoltage_V = 143.1f;
        ut.pv2InputCurrent_A = 6;
        ut.acOutputActivePower_W = 548;
        ut.batteryVoltage_V = 53.5f;
        ut.batteryChargingCurrent_A = 22;
        ut.batteryDischargeCurrent_A = 0;
        
        const std::vector<UnitTelemetry> singleTelemetry{ut};
        auto aggregated = aggregateTelemetry(singleTelemetry);
        
        // PV1: 138.9 * 6 = 833.4W, PV2: 143.1 * 6 = 858.6W, Total: 1692W
        CHECK_THAT(aggregated.solarPower_W, WithinAbs(1692.0, 0.1));
        CHECK_THAT(aggregated.acPower_W, WithinAbs(548.0, 0.01));
        // Battery: 53.5 * (22 - 0) = 1177W charging
        CHECK_THAT(aggregated.batteryPower_W, WithinAbs(1177.0, 0.1));
    }
    
    SECTION("Aggregate multiple units")
    {
        UnitTelemetry ut1;
        ut1.pv1InputVoltage_V = 100.0f;
        ut1.pv1InputCurrent_A = 5;
        ut1.pv2InputVoltage_V = 100.0f;
        ut1.pv2InputCurrent_A = 5;
        ut1.acOutputActivePower_W = 500;
        ut1.batteryVoltage_V = 50.0f;
        ut1.batteryChargingCurrent_A = 10;
        ut1.batteryDischargeCurrent_A = 0;
        
        UnitTelemetry ut2;
        ut2.pv1InputVoltage_V = 120.0f;
        ut2.pv1InputCurrent_A = 4;
        ut2.pv2InputVoltage_V = 120.0f;
        ut2.pv2InputCurrent_A = 4;
        ut2.acOutputActivePower_W = 600;
        ut2.batteryVoltage_V = 52.0f;
        ut2.batteryChargingCurrent_A = 0;
        ut2.batteryDischargeCurrent_A = 5;
        
        const std::vector<UnitTelemetry> multiTelemetry{ut1, ut2};
        auto aggregated = aggregateTelemetry(multiTelemetry);
        
        // PV1: 100*5 + 120*4 = 500+480 = 980W
        // PV2: 100*5 + 120*4 = 500+480 = 980W
        // Total solar: 1960W
        CHECK_THAT(aggregated.solarPower_W, WithinAbs(1960.0, 0.1));
        // AC: 500 + 600 = 1100W
        CHECK_THAT(aggregated.acPower_W, WithinAbs(1100.0, 0.01));
        // Battery: 50*(10-0) + 52*(0-5) = 500 - 260 = 240W
        CHECK_THAT(aggregated.batteryPower_W, WithinAbs(240.0, 0.1));
    }
}
