# GrowTrack - ESP32 Smart Garden System

This repository contains the Arduino (ESP32) firmware for **GrowTrack**, an IoT-based smart gardening and automated plant watering system.

The ESP32 device collects data from a suite of environmental sensors (temperature, humidity, soil moisture, rain, and water tank level) and transmits it to a central WebSocket server. It also listens for commands from the server to remotely activate a water pump, with a dynamic watering duration calculated based on the current soil moisture level.

## Features

* **Real-time Sensor Monitoring**: Tracks temperature, humidity, soil moisture, rain detection, and water tank level.
* **WebSocket Communication**: Connects to a server for efficient, real-time, two-way communication.
* **JSON Data Format**: Sensor data is serialized into a JSON object for easy server-side parsing.
* **Remote Water Pump Control**: Receives a `"water"` command from the server to activate the pump.
* **Smart Watering Logic**: Automatically calculates the watering duration (1, 3, or 5 seconds) based on how dry the soil is.
* **Status Feedback**: Sends a `"watered"` confirmation message back to the server after the pump runs.
* **Auto-Reconnect**: Automatically attempts to reconnect to the WebSocket server if the connection is lost.

## Hardware Requirements

* **Microcontroller**: ESP32 Development Board
* **Sensors**:
    * DHT11 (or DHT22) Temperature & Humidity Sensor
    * Capacitive Soil Moisture Sensor (Analog)
    * Raindrop Sensor / Module (Analog)
    * HC-SR04 Ultrasonic Sensor (for water level)
* **Actuator**:
    * L298N Motor Driver
    * 5V-12V DC Water Pump (or a small DC motor for testing)
* **Other**:
    * Jumper Wires
    * Breadboard
    * External power supply for the L298N and pump (recommended)

## Software & Libraries

This project is built using the Arduino IDE with the ESP32 board manager. You will need to install the following libraries via the Arduino Library Manager:

* [`WebSockets` by Markus Sattler](https://github.com/Links2004/arduinoWebSockets) (for `WebSocketsClient.h`)
* [`ArduinoJson` by Benoit Blanchon](https://github.com/bblanchon/ArduinoJson)
* [`DHT sensor library` by Adafruit](https://github.com/adafruit/DHT-sensor-library)
* [`Ultrasonic` by ErickSimGes](https://github.com/ErickSimoes/Ultrasonic)
* [`L298N` by Andrea-Vergari](https://github.com/Andrea-Vergari/L298N)

The `WiFi.h` and `HTTPClient.h` libraries are included with the standard ESP32 board installation.

## ESP32 Pinout & Wiring

| Component | ESP32 Pin |
| :--- | :--- |
| Raindrop Sensor (Analog Out) | 34 |
| Soil Moisture Sensor (Analog Out)| 35 |
| DHT11 Sensor (Data) | 2 |
| Ultrasonic Sensor (TRIG) | 18 |
| Ultrasonic Sensor (ECHO) | 19 |
| L298N Motor Driver (ENB) | 32 |
| L298N Motor Driver (IN3) | 16 |
| L298N Motor Driver (IN4) | 17 |

**Note**: Remember to provide common ground (GND) between the ESP32 and the L298N driver. The L298N should be powered separately (e.g., via `12V` and `GND` pins) to avoid drawing too much current from the ESP32.



## How It Works

### 1. Initialization (`setup()`)
1.  Starts the Serial Monitor.
2.  Initializes the DHT sensor and sets the L298N motor speed.
3.  Connects to the specified WiFi network.
4.  Establishes a WebSocket connection to the server at `ws://<serverIP>:<serverPort><endPoint>`.
5.  Sets up an event handler (`webSocketEvent`) for WebSocket messages.

### 2. Main Loop (`loop()`)
1.  Keeps the WebSocket connection alive using `socket.loop()`.
2.  Every 5 seconds:
    * Calls `getSensorData()` to read all sensors.
    * The sensor data is packed into a JSON string, e.g., `{"raindrop":3000,"soilMoisture":2500,"waterLevel":10,"temperature":28.5,"humidity":65.0}`.
    * This JSON string is sent to the server with the prefix `sensorData:`.
    * A WebSocket `ping` is sent to keep the connection active.

### 3. WebSocket Events (`webSocketEvent()`)
* **`WStype_CONNECTED`**: Prints a connection confirmation to the Serial Monitor.
* **`WStype_DISCONNECTED`**: Prints a disconnection message.
* **`WStype_TEXT`**: This is the command handler.
    1.  It checks if the received text message from the server is exactly `"water"`.
    2.  If it is, it reads the soil moisture, maps it to a 0-100% scale, and calls `calculateWateringTime()`.
    3.  `calculateWateringTime()` returns a duration based on the soil moisture percentage (1, 3, or 5 seconds).
    4.  The motor is activated (`motor.forward()`) for the calculated duration.
    5.  The motor is stopped (`motor.stop()`).
    6.  A confirmation message, `"watered"`, is sent back to the server.

## Setup & Installation

1.  **Clone the Repository**:
    ```sh
    git clone [https://github.com/YOUR_USERNAME/YOUR_REPOSITORY.git](https://github.com/YOUR_USERNAME/YOUR_REPOSITORY.git)
    ```
2.  **Hardware Setup**: Connect all the sensors and modules to the ESP32 as shown in the [Pinout Table](#esp32-pinout--wiring).
3.  **Arduino IDE**:
    * Open the `.ino` sketch file in the Arduino IDE.
    * Make sure you have the **ESP32 Board Manager** installed.
    * Go to **Tools > Library Manager** and install all the libraries listed in the [Software & Libraries](#software--libraries) section.
    * Select your ESP32 board (e.g., "ESP32 Dev Module") and the correct COM port.
4.  **Configuration**: Before uploading, you **must** modify the following constants in the code to match your setup:

    ```cpp
    // --- WiFi Credentials ---
    const char* ssid = "YOUR_WIFI_SSID";
    const char* password = "YOUR_WIFI_PASSWORD";

    // --- Server Details ---
    const char* serverIP = "192.168.1.100"; // IP address of your WebSocket server
    const uint16_t serverPort = 8080;         // Port your server is running on
    const char* endPoint = "/GrowTrack/LoadData?type=esp"; // Your server's endpoint

    // --- Sensor Calibration ---
    const int tankHeight = 14; // Total height of your water tank in cm
    ```
5.  **Upload**: Click the "Upload" button in the Arduino IDE.
6.  **Monitor**: Open the Serial Monitor (Baud Rate: 115200) to see connection status, sensor data, and event logs.

## Server-Side Communication Protocol

### ESP32 to Server
* **Sensor Data**: The ESP32 sends a message prefixed with `sensorData:` followed by a JSON string.
    ```
    sensorData:{"raindrop":3000,"soilMoisture":2500,"waterLevel":10,"temperature":28.5,"humidity":65.0}
    ```
* **Watering Confirmation**: After successfully watering, the ESP32 sends:
    ```
    watered
    ```

### Server to ESP32
* **Water Command**: To trigger the water pump, the server must send a plain text message:
    ```
    water
    ```

## Frontend

```bash
https://github.com/DisuraAberathna/GrowTrack_Front-End
```
## Backend

```bash
https://github.com/DisuraAberathna/GrowTrack_Back-End
```
