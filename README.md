# CDP SOLAX Daemon

This program connects to a CDP solar inverter / charger through the serial port and reads out its telemetry. A REST service is running to provide that information to a consumer such as [Home Assistant](https://www.home-assistant.io/).

## Hardware Setup

This is an example setup that this program has been developed for. 

 * Raspberry Pi 2B @ Rasbian 12
 * Altered OIKWAN Console Cable USB-C TO RJ45 (RS-232)
 * CDP ES-6548 SOLAX solar inverter / battery charger

## Building

### Installing dependencies

Run the following commands to install the dependencies.

```
sudo apt install libcpprest-dev binutils-dev cmake

cd /tmp
git clone https://github.com/gbmhunter/CppLinuxSerial.git
cd CppLinuxSerial
mkdir build
cd build
cmake -DSERIAL_BUILD_SHARED_LIBS=ON ..
make -j4
sudo make install
```

### Compiling solax daemon

```
git clone https://github.com/Hemofektik/cdp_solax_daemon.git
cd cdp_solax_daemon
mkdir build
cd build
cmake ..
make -j4
```

## Configuration

Update the REST server port or the device path for the CDP SOLAX BMS UART interface in the [solax.cfg](solax.cfg) to your needs.

## Running as a service/daemon

Once compilation has been completed successfully run the following commands in the build directory to enable the solax service.

```
sudo make install
sudo ldconfig
sudo systemctl enable solax
sudo systemctl start solax
```

# Home Assistant Integration

Extend the [configuration.yaml](https://www.home-assistant.io/docs/configuration/) as follows and [restart Home Assistant](https://www.home-assistant.io/docs/configuration/#reloading-the-configuration-to-apply-changes).

```
rest:
  - scan_interval: 1
    resource: http://localhost:5074/telemetry/aggregated
    sensor:
      - name: "Solar Power"
        unique_id: "SOLAX_SOLAR_POWER"
        value_template: "{{ value_json['solarPower_W'] }}"
        device_class: power
        unit_of_measurement: "W"
      - name: "AC Power"
        unique_id: "SOLAX_AC_POWER"
        value_template: "{{ value_json['acPower_W'] }}"
        device_class: power
        unit_of_measurement: "W"
      - name: "Battery Power"
        unique_id: "SOLAX_BATTERY_POWER"
        value_template: "{{ value_json['batteryPower_W'] }}"
        device_class: power
        unit_of_measurement: "W"
```