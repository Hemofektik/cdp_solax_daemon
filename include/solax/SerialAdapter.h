#pragma once
#include <CppLinuxSerial/SerialPort.hpp>

namespace solax
{

class SerialAdapter final
{
public:
    struct Config
    {
        std::vector<std::string> devicePaths{"/dev/ttyUSB0"};
        mn::CppLinuxSerial::BaudRate baudRate{mn::CppLinuxSerial::BaudRate::B_2400};
        mn::CppLinuxSerial::NumDataBits numDataBits{mn::CppLinuxSerial::NumDataBits::EIGHT};
        mn::CppLinuxSerial::Parity parity{mn::CppLinuxSerial::Parity::NONE};
        mn::CppLinuxSerial::NumStopBits numStopBits{mn::CppLinuxSerial::NumStopBits::ONE};
        mn::CppLinuxSerial::HardwareFlowControl hardwareFlowControl{mn::CppLinuxSerial::HardwareFlowControl::OFF};
        mn::CppLinuxSerial::SoftwareFlowControl softwareFlowControl{mn::CppLinuxSerial::SoftwareFlowControl::OFF};
    };

    SerialAdapter(const Config& config);
    ~SerialAdapter();

	SerialAdapter(SerialAdapter const &) = delete;
	SerialAdapter &operator=(SerialAdapter const &) = delete;
	SerialAdapter(SerialAdapter &&) = delete;
	SerialAdapter &operator=(SerialAdapter &&) = delete;
    
    std::string readRawTelemetry(uint8_t machineIndex);

private:
    mn::CppLinuxSerial::SerialPort serialPort;
    std::string activeDevicePath;
};


}