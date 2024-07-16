# Smart Water Level Management System

This project involves an ESP32-based Smart Water Level Management System that monitors the level of water (or any liquid) in a vessel. The system uses an ultrasonic sensor to measure the water level, an OLED display to show the level, and a buzzer for alerts. Additionally, the data is sent to servers for remote monitoring.

## Table of Contents
1. [Project Overview](#project-overview)
2. [Components](#components)
3. [Circuit Diagram](#circuit-diagram)
4. [Pin Configuration](#pin-configuration)
5. [Installation and Setup](#installation-and-setup)
6. [Code](#code)
7. [Usage](#usage)

## Project Overview
The Smart Water Level Management System is designed to:
- Measure the water level using an HC-SR04 ultrasonic sensor.
- Display the water level on a 0.96 inch OLED display.
- Alert when the water level falls below 5% or rises above 95% of the vessel's capacity using a buzzer.
- Transmit the data to servers for remote monitoring.

## Components
- ESP32 Development Module
- HC-SR04 Ultrasonic Sensor
- 0.96 inch OLED Display (SSD1306 controller)
- Buzzer

## Circuit Diagram
The Fritzing model of the circuit can be found [here](https://github.com/ZAVESKO/Smart-Water-Level-System-IOT-/blob/main/Circuit.fzz).

## Pin Configuration
### Buzzer
- Positive (long leg) -> GPIO 19
- Negative (short leg) -> GND

### HC-SR04 Ultrasonic Sensor
- VCC -> 5V
- GND -> GND
- TRIG -> GPIO 5
- ECHO -> GPIO 18

### OLED Display
- VCC -> 3.3V
- GND -> GND
- SCL -> GPIO 22
- SDA -> GPIO 21

## Installation and Setup
1. **Clone the Repository:**
    ```sh
    git clone https://github.com/ZAVESKO/Smart-Water-Level-System-IOT-.git
    ```
2. **Open the Code:**
    The complete code for the project is available [here](https://github.com/ZAVESKO/Smart-Water-Level-System-IOT-/blob/main/code.ino). Open the file in your preferred IDE.

3. **Upload the Code:**
    Ensure you have the required libraries installed. Connect your ESP32 to your computer and upload the code.

## Code
The code handles:
- Reading the water level using the ultrasonic sensor.
- Displaying the level on the OLED display.
- Triggering the buzzer when the water level falls below 5% or rises above 95%.
- Sending the data to servers for remote monitoring.

## Usage
1. **Mount the Ultrasonic Sensor:**
    Place the ultrasonic sensor at the top of the vessel to measure the water level accurately.

2. **Power the System:**
    Connect the ESP32 to a power source.

3. **Monitor the Display:**
    The OLED display will show the current water level.

4. **Alerts:**
    The buzzer will alert you if the water level is too low (below 5%) or too high (above 95%).
