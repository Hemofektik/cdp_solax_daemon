#include <solax/SerialAdapter.h>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;
using namespace mn::CppLinuxSerial;

namespace solax
{

SerialAdapter::SerialAdapter(const Config& config)
: serialPort("", config.baudRate, config.numDataBits, config.parity, config.numStopBits, config.hardwareFlowControl, config.softwareFlowControl)
{
    bool found = false;
    for(const auto& path : config.devicePaths) 
    {
        std::cout << "Checking device: "  << path << std::endl;

        try 
        {
            serialPort.SetDevice(path);
            serialPort.SetTimeout(100); // Block for up to 100ms to receive data
            serialPort.Open();

            if(serialPort.GetState() != mn::CppLinuxSerial::State::OPEN) 
            {
                std::cout << "Cannot open device: "  << path << std::endl;
                continue;
            }

            // Send a empty query to test if device responds
            serialPort.Write("\n\r");
            std::this_thread::sleep_for(500ms);
            
            std::string readData;
            auto const timeout{2000ms};
            auto const stepDuration{100ms};
            auto waitedFor{0ms};
            
            while(waitedFor < timeout) 
            {
                if(serialPort.Available()) 
                {
                    serialPort.Read(readData);
                    
                    // Check if response contains '(' which indicates proper protocol response
                    if(readData.find("(NAKss") != std::string::npos) 
                    {
                        found = true;
                        activeDevicePath = path;
                        break;
                    }
                }
                std::this_thread::sleep_for(stepDuration);
                waitedFor += stepDuration;
            }
            serialPort.Close();
            if(found) 
            {
                std::cout << "Found CDP SOLAX device at: "  << path << std::endl;
                std::this_thread::sleep_for(200ms); // Give device time to settle
                serialPort.SetDevice(path);
                serialPort.SetTimeout(100);
                serialPort.Open();
                break;
            }
        }
        catch(const mn::CppLinuxSerial::Exception&) 
        {
            // Try next device
            try { serialPort.Close(); } catch(...) {}
        }
    }
    if(!found) 
    {
        throw std::runtime_error("No serial device responded with the expected protocol format (no '(NAKss' found in response)");
    }
}

SerialAdapter::~SerialAdapter()
{
    try
    {
        serialPort.Close();
    }
    catch(const mn::CppLinuxSerial::Exception& e)
    {
        // ignore errors here since we are done with the serial port
    }
}

std::string SerialAdapter::readRawTelemetry(uint8_t machineIndex)
{
    std::string_view const randomCrc{"34"};
    char const messageStartToken{'('};
    char const messageEndToken{'\r'};
    auto const timeout{5000ms};
    auto const stepDuration{100ms};
    auto waitedFor{0ms};

    try
    {
        // Flush any pending data from the serial port
        while(serialPort.Available())
        {
            std::string discardData;
            serialPort.Read(discardData);
        }
        
        serialPort.Write("QPGS");
        serialPort.Write(std::to_string(machineIndex));
        serialPort.Write(randomCrc.data());
        serialPort.Write("\r");

        std::string totalReadData;
        auto startTime = std::chrono::steady_clock::now();
        
        while(std::chrono::steady_clock::now() - startTime < timeout)
        {
            if(serialPort.Available())
            {
                std::string readData;
                serialPort.Read(readData);
                totalReadData.append(readData);
                
                // Check if we have a complete message
                auto const start{totalReadData.find(messageStartToken)};
                auto const end{totalReadData.find(messageEndToken, start)};
                
                if(start != std::string::npos && end != std::string::npos && end > start)
                {
                    // Extract message between '(' and '\r'
                    return totalReadData.substr(start + 1, end - start - 1);
                }
            }
            
            std::this_thread::sleep_for(stepDuration);
        }
        
        // Timeout - check if we got any data at all
        if(!totalReadData.empty())
        {
            throw std::runtime_error("Incomplete response from device: " + totalReadData);
        }
    }
    catch(const mn::CppLinuxSerial::Exception& e)
    {
        throw std::runtime_error(std::string("Serial Device Error: ") + e.what());
    }

    throw std::runtime_error("Timeout occured! Did not receive data from the serial device.");
}

}