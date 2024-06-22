#include "ACS712.h"

#define SOLAR_PANEL_VOLTAGE_PIN A0 // Analog pin for solar panel voltage
#define BATTERY_VOLTAGE_PIN A1     // Analog pin for battery voltage
#define LOAD_VOLTAGE_PIN A2         // Analog pin for load voltage
#define SOLAR_CURRENT_PIN A3        // Analog pin for solar current
#define LOAD_CURRENT_PIN A4         // Analog pin for load current
#define TEMPERATURE_PIN A5          // Analog pin for LM35 temperature sensor
#define SOLAR_RELAY_PIN 6          // Pin connected to the relay for solar side
#define LOAD_RELAY_PIN 7           // Pin connected to the relay for load side
#define MOSFET_PIN 10               // Pin connected to the gate of P-channel MOSFET

// Thresholds for various parameters
float MIN_BATTERY_VOLTAGE = 11.5; // Minimum battery voltage
float MAX_BATTERY_VOLTAGE = 14.5; // Maximum battery voltage
float MAX_SOLAR_CURRENT = 5.0;    // Maximum current from solar panel
float MAX_LOAD_CURRENT = 10.0;    // Maximum current for load
float MAX_TEMPERATURE = 50.0;     // Maximum temperature in Celsius

bool debugMode = false; // Debug mode flag

void setup()
{
  Serial.begin(9600);
  pinMode(SOLAR_RELAY_PIN, OUTPUT);
  pinMode(LOAD_RELAY_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
}

void loop()
{
  float solarPanelVoltage = measureVoltage(SOLAR_PANEL_VOLTAGE_PIN);
  float batteryVoltage = measureVoltage(BATTERY_VOLTAGE_PIN);
  float loadVoltage = measureVoltage(LOAD_VOLTAGE_PIN);
  float solarCurrent = measureCurrent(SOLAR_CURRENT_PIN);
  float loadCurrent = measureCurrent(LOAD_CURRENT_PIN);
  float temperature = measureTemperature(TEMPERATURE_PIN);

  // Control logic for overvoltage, undervoltage, overcurrent, and temperature protection
  if (batteryVoltage > MAX_BATTERY_VOLTAGE || solarPanelVoltage > MAX_BATTERY_VOLTAGE || loadVoltage > MAX_BATTERY_VOLTAGE ||
      batteryVoltage < MIN_BATTERY_VOLTAGE || solarCurrent > MAX_SOLAR_CURRENT || loadCurrent > MAX_LOAD_CURRENT || temperature > MAX_TEMPERATURE)
  {
    if (debugMode)
    {
      Serial.println("System Protection Triggered!");
      Serial.println("Turning off relays and MOSFET...");
    }
    digitalWrite(SOLAR_RELAY_PIN, LOW); // Turn off solar relay
    digitalWrite(LOAD_RELAY_PIN, LOW);  // Turn off load relay
    analogWrite(MOSFET_PIN, 0);         // Set PWM to 0 to disable charging current
  }
  else
  {
    if (debugMode)
    {
      Serial.println("System Running Normally.");
    }
    // Control logic for solar side relay based on battery voltage and solar panel voltage
    if (batteryVoltage < MAX_BATTERY_VOLTAGE && solarPanelVoltage > batteryVoltage)
    {
      digitalWrite(SOLAR_RELAY_PIN, HIGH);                                              // Turn on solar relay to allow charging
      analogWrite(MOSFET_PIN, map(solarPanelVoltage, batteryVoltage, 20, 0, 255)); // Control charging current using PWM
    }
    else
    {
      digitalWrite(SOLAR_RELAY_PIN, LOW); // Turn off solar relay to stop charging
      analogWrite(MOSFET_PIN, 0);         // Set PWM to 0 to disable charging current
    }

    // Control logic for load side relay based on battery voltage and load current
    if (batteryVoltage > MIN_BATTERY_VOLTAGE && loadCurrent < MAX_LOAD_CURRENT)
    {
      digitalWrite(LOAD_RELAY_PIN, HIGH); // Turn on load relay to supply power to load
    }
    else
    {
      digitalWrite(LOAD_RELAY_PIN, LOW); // Turn off load relay to prevent over-discharging or overload
    }
  }

  // Debug mode control via Serial commands
  if (Serial.available() > 0)
  {
    char command = Serial.read();
    if (command == 'd' || command == 'D')
    {
      debugMode = !debugMode;
      if (debugMode)
      {
        Serial.println("Debug mode enabled.");
      }
      else
      {
        Serial.println("Debug mode disabled.");
      }
    }
    else if (command == 's' || command == 'S')
    {
      // Send sensor readings
      printSensorReadings(solarPanelVoltage, batteryVoltage, loadVoltage, solarCurrent, loadCurrent, temperature);
    }
    else if (command == 't' || command == 'T')
    {
      // Change threshold values
      changeThresholdValues();
    }
  }

  
}

void printSensorReadings(float solarPanelVoltage, float batteryVoltage, float loadVoltage, float solarCurrent, float loadCurrent, float temperature)
{
  Serial.println("Sensor Readings:");
  Serial.print("Solar Panel Voltage: ");
  Serial.print(solarPanelVoltage);
  Serial.println(" V");
  Serial.print("Battery Voltage: ");
  Serial.print(batteryVoltage);
  Serial.println(" V");
  Serial.print("Load Voltage: ");
  Serial.print(loadVoltage);
  Serial.println(" V");
  Serial.print("Solar Current: ");
  Serial.print(solarCurrent);
  Serial.println(" A");
  Serial.print("Load Current: ");
  Serial.print(loadCurrent);
  Serial.println(" A");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
}

void changeThresholdValues()
{
  Serial.println("Enter new threshold values (Format: minBattery,maxBattery,maxSolarCurrent,maxLoadCurrent,maxTemperature):");
  while (!Serial.available())
  {
    // Wait for user input
  }

  String input = Serial.readStringUntil('\n');
  int pos = input.indexOf(',');
  if (pos != -1)
  {
    String minBatteryStr = input.substring(0, pos);
    input.remove(0, pos + 1);
    pos = input.indexOf(',');
    if (pos != -1)
    {
      String maxBatteryStr = input.substring(0, pos);
      input.remove(0, pos + 1);
      pos = input.indexOf(',');
      if (pos != -1)
      {
        String maxSolarCurrentStr = input.substring(0, pos);
        input.remove(0, pos + 1);
        pos = input.indexOf(',');
        if (pos != -1)
        {
          String maxLoadCurrentStr = input.substring(0, pos);
          input.remove(0, pos + 1);
          String maxTemperatureStr = input;

          // Convert strings to floats
          MIN_BATTERY_VOLTAGE = minBatteryStr.toFloat();
          MAX_BATTERY_VOLTAGE = maxBatteryStr.toFloat();
          MAX_SOLAR_CURRENT = maxSolarCurrentStr.toFloat();
          MAX_LOAD_CURRENT = maxLoadCurrentStr.toFloat();
          MAX_TEMPERATURE = maxTemperatureStr.toFloat();

          Serial.println("Threshold values updated.");
          return;
        }
      }
    }
  }

  Serial.println("Invalid input. Please try again.");
}

float measureVoltage(int pin)
{
  int sensorValue = analogRead(pin);
  float voltage = sensorValue * (5.0 / 1023.0) * (3.0); // Adjusted for potential divider
  return voltage;
}

float measureCurrent(int x)
{
  
  ACS712 Ca(x,5.0, 1023, 185); // Change this according to your ACS712 model
  
  int current = Ca.mA_DC(); // Read current from ACS712 sensor
  return current;
}

float measureTemperature(int pin)
{
  int sensorValue = analogRead(pin);
  float milliVolts = sensorValue * (5000.0 / 1024.0);
  float temperature = milliVolts / 10.0;
  return temperature;
}
