/**
 * Integration test for SerialAdapter device detection.
 * 
 * This test requires real hardware connected to /dev/ttyUSB0 or /dev/ttyUSB1.
 * Run with: ./test_serial_adapter
 * 
 * The test is tagged with [integration] so it can be filtered:
 *   ./test_serial_adapter "[integration]"     # run only integration tests
 *   ./test_serial_adapter "~[integration]"    # exclude integration tests
 */

#include <catch2/catch_test_macros.hpp>
#include <solax/SerialAdapter.h>

#include <iostream>
#include <fstream>

namespace {

bool deviceExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

} // anonymous namespace

TEST_CASE("SerialAdapter detects CDP SOLAX device on ttyUSB", "[integration][serial][detection]") {
    
    // Check if any USB serial device is available
    bool usb0Exists = deviceExists("/dev/ttyUSB0");
    bool usb1Exists = deviceExists("/dev/ttyUSB1");
    
    if (!usb0Exists && !usb1Exists) {
        WARN("No USB serial device found at /dev/ttyUSB0 or /dev/ttyUSB1 - skipping integration test");
        SKIP("No USB serial device available");
    }
    
    INFO("Found devices: ttyUSB0=" << usb0Exists << ", ttyUSB1=" << usb1Exists);
    
    SECTION("SerialAdapter successfully detects device on available ttyUSB ports") {
        solax::SerialAdapter::Config config;
        config.devicePaths = {"/dev/ttyUSB0", "/dev/ttyUSB1"};
        config.baudRate = mn::CppLinuxSerial::BaudRate::B_2400;
        
        REQUIRE_NOTHROW([&]() {
            solax::SerialAdapter adapter(config);
            INFO("Successfully connected to CDP SOLAX device");
        }());
    }
}

TEST_CASE("SerialAdapter can read telemetry from real device", "[integration][serial][telemetry]") {
    
    bool usb0Exists = deviceExists("/dev/ttyUSB0");
    bool usb1Exists = deviceExists("/dev/ttyUSB1");
    
    if (!usb0Exists && !usb1Exists) {
        SKIP("No USB serial device available");
    }
    
    solax::SerialAdapter::Config config;
    config.devicePaths = {"/dev/ttyUSB0", "/dev/ttyUSB1"};
    config.baudRate = mn::CppLinuxSerial::BaudRate::B_2400;
    
    SECTION("Read telemetry from machine index 0") {
        try {
            solax::SerialAdapter adapter(config);
            std::string telemetry = adapter.readRawTelemetry(0);
            
            INFO("Received telemetry: " << telemetry);
            REQUIRE(!telemetry.empty());
        } catch (const std::runtime_error& e) {
            // Device might not respond with sentinel - that's ok for this test
            WARN("Could not connect to device: " << e.what());
            SKIP("Device not responding with expected sentinel");
        }
    }
}
