# ESP8266 Distance Monitor with WebSocket Streaming

A standalone distance measurement system using HC-SR04 ultrasonic sensor with real-time WebSocket streaming to a web interface. The NodeMCU ESP8266 creates its own WiFi access point.

## Features

- Real-time distance measurement (2cm-400cm range)
- WebSocket streaming for live updates
- Responsive web interface
- Toggle between centimeters and inches
- Self-contained access point mode (no WiFi router needed)

## Hardware Requirements

- NodeMCU ESP8266
- HC-SR04 Ultrasonic Sensor
- Breadboard and jumper wires
- 220Ω and 470Ω resistors (for voltage divider)

## Circuit Diagram

HC-SR04 NodeMCU
VCC Vin or 5V
Trig D1 (GPIO5)
Echo D2 (GPIO4) - with voltage divider
GND GND


## Installation

### Arduino IDE Method

1. Install ESP8266 board package:
   - File > Preferences > Additional Boards Manager URLs:
     `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
   - Tools > Board > Boards Manager > Search for "esp8266" and install

2. Select board:
   - Tools > Board > NodeMCU 1.0 (ESP-12E Module)
   - Set Upload Speed: "115200"

3. Upload the sketch from `firmware/ESP8266_Distance_Monitor.ino`

### PlatformIO Method (recommended)

1. Install [PlatformIO IDE](https://platformio.org/platformio-ide)
2. Open project folder
3. Build and upload

## Usage

1. After uploading, the ESP8266 will create a WiFi network "DistanceMonitorAP"
2. Connect to this network (password: "measure123")
3. Open a web browser and navigate to: `http://192.168.4.1`
4. The distance will update in real-time

## Customization

- Change AP credentials in the sketch:
  ```cpp
  const char *ssid = "DistanceMonitorAP";
  const char *password = "measure123";
  
## Modify update frequency (milliseconds):

if (millis() - lastUpdate > 100) { // Change this value

## Libraries Used
ESP8266WiFi

WebSocketsServer

ESP8266WebServer

License
MIT License - See LICENSE file for details

