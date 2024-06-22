Solar Management System
This repository contains the code for a Solar Management System that controls and monitors a solar panel, battery, and DC load. It uses an Arduino or ESP32 board for controlling relays and monitoring various parameters such as voltage, current, and temperature. The system also features an optional integration with the Blynk app for remote monitoring and control.

Features:

Voltage Monitoring: Monitors solar panel, battery, and load voltages.
Current Monitoring: Monitors the current from the solar panel and the load.
Temperature Monitoring: Monitors the temperature using a temperature sensor.
Relay Control: Controls relays for connecting/disconnecting the solar panel and the load.
MOSFET Control: Uses PWM to control the charging current from the solar panel.
Overvoltage, Undervoltage, Overcurrent, and Temperature Protection: Implements protection mechanisms to safeguard the system components.
Blynk Integration: Allows remote monitoring and control via the Blynk app.
Debug Mode: Enables detailed logging for troubleshooting.
Hardware Requirements
Microcontroller: Arduino or ESP32
Voltage Sensors: For measuring solar panel, battery, and load voltages.
Current Sensors: ACS712 or similar for measuring current.
Temperature Sensor: Analog temperature sensor.
Relays: For switching the solar panel and load connections.
MOSFET: For controlling the charging current from the solar panel.
Optional: Wi-Fi module for Arduino if using Blynk with Arduino.

Software Requirements:

Arduino IDE: For programming the Arduino or ESP32.
Blynk Library: For integrating with the Blynk app.
WiFi Library: For Wi-Fi connectivity (ESP32/Arduino with Wi-Fi module).

Installation:

Clone the Repository:

bash
Copy code
git clone https://github.com/TirthChauhan1304/solarManagementSystem.git

Install Arduino IDE: 

Download and install the Arduino IDE.

Install Libraries:

Blynk Library
Go to Sketch > Include Library > Manage Libraries.
Search for Blynk and install the latest version.
WiFi Library (if using ESP32/Arduino with Wi-Fi module):
Go to Sketch > Include Library > Manage Libraries.
Search for WiFi and install the latest version.

Open the Project:

Open the Arduino IDE.
Navigate to File > Open and select the solar_management.ino file from the cloned repository.
Configure Wi-Fi and Blynk Credentials:

In the code, replace the placeholder values with your Wi-Fi SSID, password, and Blynk authentication token.

Upload the Code:

Connect your Arduino/ESP32 to your computer.
Select the correct board and port from Tools.
Click the Upload button to upload the code to your microcontroller.

Usage:

Monitor Parameters: Use the serial monitor to view real-time data for voltage, current, and temperature.
Remote Monitoring: Open the Blynk app on your smartphone to monitor and control the system remotely.
Debug Mode: Send debug command via the serial monitor to enable or disable debug mode. When enabled, detailed logs will be printed.
Customization
Threshold Values: Adjust the threshold values for voltage, current, and temperature protection in the code.
Adding New Sensors: Extend the code to include additional sensors as needed.
Additional Features: Implement new features based on your specific requirements.

Acknowledgements:
This project was made possible by the following resources and contributions:

Blynk Library:
WiFi Library for ESP32
Arduino Core for ESP32

License:
This project is licensed under the MIT License - see the LICENSE file for details.
