
#include <string_view>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <solax/Telemetry.h>

const std::string_view solaxOutput = 
"1 96342304101107 B 00 000.0 00.00 110.3 60.01 0569 0548 008 53.5 022 093 138.9 041 01496 01445 011 10100110 5 3 100 120 040 06 000 143.1 06�k";

using namespace solax;
using Catch::Matchers::WithinAbs;

SCENARIO( "QPGSn Telemetry can be parsed", "[solax::telemetry]" ) 
{
    GIVEN( "raw telemetry from Solax QPGSn command" ) 
    {
        const std::string telemetry{solaxOutput};

        WHEN( "parsing the telemetry" ) 
        {
            auto const parsed = solax::parseRawTelemetry(telemetry);

            THEN( "Basic device info is parsed correctly" ) 
            {
                REQUIRE( parsed.parallelNum == 1 );
                REQUIRE( parsed.serialNumber == "96342304101107" );
                REQUIRE( parsed.workMode == 'B' );
                REQUIRE( parsed.faultCode == 0 );
            }
            AND_THEN( "Grid parameters are parsed correctly" )
            {
                REQUIRE_THAT( parsed.gridVoltage_V, WithinAbs(0.0, 0.01) );
                REQUIRE_THAT( parsed.gridFrequency_Hz, WithinAbs(0.0, 0.01) );
            }
            AND_THEN( "AC output parameters are parsed correctly" )
            {
                REQUIRE_THAT( parsed.acOutputVoltage_V, WithinAbs(110.3, 0.01) );
                REQUIRE_THAT( parsed.acOutputFrequency_Hz, WithinAbs(60.01, 0.01) );
                REQUIRE( parsed.acOutputApparentPower_VA == 569 );
                REQUIRE( parsed.acOutputActivePower_W == 548 );
                REQUIRE( parsed.loadPercent == 8 );
            }
            AND_THEN( "Battery parameters are parsed correctly" )
            {
                REQUIRE_THAT( parsed.batteryVoltage_V, WithinAbs(53.5, 0.01) );
                REQUIRE( parsed.batteryChargingCurrent_A == 22 );
                REQUIRE( parsed.batteryCapacity_pct == 93 );
                REQUIRE( parsed.batteryDischargeCurrent_A == 0 );
            }
            AND_THEN( "PV parameters are parsed correctly" )
            {
                REQUIRE_THAT( parsed.pv1InputVoltage_V, WithinAbs(138.9, 0.01) );
                REQUIRE( parsed.pv1InputCurrent_A == 6 );
                REQUIRE_THAT( parsed.pv2InputVoltage_V, WithinAbs(143.1, 0.01) );
                REQUIRE( parsed.pv2InputCurrent_A == 6 );
            }
            AND_THEN( "Total/aggregate values are parsed correctly" )
            {
                REQUIRE( parsed.totalChargingCurrent_A == 41 );
                REQUIRE( parsed.totalAcOutputApparentPower_VA == 1496 );
                REQUIRE( parsed.totalOutputActivePower_W == 1445 );
                REQUIRE( parsed.totalAcOutputPercent == 11 );
            }
            AND_THEN( "Mode and configuration values are parsed correctly" )
            {
                REQUIRE( parsed.inverterStatus == "10100110" );
                REQUIRE( parsed.outputMode == 5 );
                REQUIRE( parsed.chargerSourcePriority == 3 );
                REQUIRE( parsed.maxChargerCurrent_A == 100 );
                REQUIRE( parsed.maxChargerRange_A == 120 );
                REQUIRE( parsed.maxAcChargerCurrent_A == 40 );
            }
        }
    }
}
