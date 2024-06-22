#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""

///* Comment this out to disable prints and save space */
//#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h> // Include the Blynk library

#define SOLAR_RELAY_PIN 21 // Pin connected to the relay for solar side
#define LOAD_RELAY_PIN 22  // Pin connected to the relay for load side
#define MOSFET_PIN 23       // Pin connected to the gate of P-channel MOSFET

int Vsensor1, Vsensor2, Vsensor3, temp, X;
float voltage1, voltage2, voltage3, temp1 ,milliVolt;
float r1 = 47000.0;
float r2 = 16500.0;
unsigned int x = 0;
float AcsValue = 0.0,AcsValueF = 0.0,C1;
float AcsValue1 = 0.0,AcsValueF1 = 0.0,C2;

//PWM paarameters
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

// Thresholds for various parameters
float MIN_BATTERY_VOLTAGE = 11.5; // Minimum battery voltage
float MAX_BATTERY_VOLTAGE = 12.4; // Maximum battery voltage
float MAX_SOLAR_CURRENT = 0.1;    // Maximum current from solar panel
float MAX_LOAD_CURRENT = 10.0;    // Maximum current for load
float MAX_TEMPERATURE = 50.0;     // Maximum temperature in Celsius

bool debugMode = false; // Debug mode flag

char auth[] = "BLYNK_AUTH_TOKEN";  // Your Blynk authentication token
char ssid[] = "";   // Your WiFi SSID
char pass[] = "";       // Your WiFi password

BlynkTimer timer;

// Attach virtual serial terminal to Virtual Pin V6
WidgetTerminal terminal(V6);

void setup()
{
  pinMode(34, INPUT);
  pinMode(35, INPUT);
  pinMode(32, INPUT);
  pinMode(33, INPUT);
  pinMode(25, INPUT);
  pinMode(26, INPUT);
  pinMode(2,  INPUT);                //debug pin
  pinMode(SOLAR_RELAY_PIN, OUTPUT);
  pinMode(LOAD_RELAY_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(MOSFET_PIN, ledChannel);

    Serial.begin(115200);
    Serial.println(" Solar Management System V 1.0.0 ");

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Initialize Blynk with your authentication token and Wi-Fi credentials
    timer.setInterval(1000L, myTimer);

    terminal.clear();

  // This will print Blynk Software version to the Terminal Widget when
  // your hardware gets connected to Blynk Server
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("-------------"));
  terminal.flush();
    
 }

void loop()
{
 
  Vsensor1 = analogRead(34); //Solar Pannel
  Vsensor2 = analogRead(35); //Battery
  Vsensor3 = analogRead(32); //Dc Load
  temp = analogRead(33);     //Temprature Sensor
  
  milliVolt = temp * (5000/4096);
  // convert the voltage to the temperature in 
  temp1 = milliVolt / 10;

  voltage1 = Vsensor1 * (3.30 / 4096.0) * ((r1 + r2) / r2);
  voltage2 = Vsensor2 * (3.30 / 4096.0) * ((r1 + r2) / r2);
  voltage3 = Vsensor3 * (3.30 / 4096.0) * ((r1 + r2) / r2);

  AcsValue = analogRead(25);        //Read current sensor values
  AcsValue1 = analogRead(26);       //Read current sensor values

  C1 = AcsValue*(3.30/4096);
  C2 = AcsValue1*(3.30/4096);
  
  AcsValueF = (C1 - 2.50)/0.100;
  AcsValueF1 = (C2 - 2.50)/0.100;

 
  // Control logic for overvoltage, undervoltage, overcurrent, and temperature protection
  if (voltage2 > MAX_BATTERY_VOLTAGE || voltage1 > MAX_BATTERY_VOLTAGE || voltage3 > MAX_BATTERY_VOLTAGE ||
      voltage2 < MIN_BATTERY_VOLTAGE || AcsValueF > MAX_SOLAR_CURRENT || AcsValueF1 > MAX_LOAD_CURRENT || temp1 > MAX_TEMPERATURE)
  {
    if (debugMode)
    {
      Serial.println("System Protection Triggered!");
      Serial.println("Turning off relays and MOSFET...");
    }
    digitalWrite(SOLAR_RELAY_PIN, HIGH); // Turn off solar relay
    digitalWrite(LOAD_RELAY_PIN, HIGH);  // Turn off load relay
    ledcWrite(ledChannel, 0);         // Set PWM to 0 to disable charging current
  }
  else
  {
    if (debugMode)
    {
      Serial.println("System Running Normally.");
    }
    // Control logic for solar side relay based on battery voltage and solar panel voltage
    if (voltage2 < MAX_BATTERY_VOLTAGE && voltage1 > voltage2)
    {
      digitalWrite(SOLAR_RELAY_PIN, LOW);                           // Turn on solar relay to allow charging
      ledcWrite(ledChannel, map(voltage1, voltage2, 20, 0, 255));   // Control charging current using PWM
       
    }
    else
    {
      digitalWrite(SOLAR_RELAY_PIN, HIGH); // Turn off solar relay to stop charging
      ledcWrite(ledChannel, 0);         // Set PWM to 0 to disable charging current
    }

    // Control logic for load side relay based on battery voltage and load current
    if (voltage2 > MIN_BATTERY_VOLTAGE && AcsValueF1 < MAX_LOAD_CURRENT)
    {
      digitalWrite(LOAD_RELAY_PIN, LOW); // Turn on load relay to supply power to load
    }
    else
    {
      digitalWrite(LOAD_RELAY_PIN, HIGH); // Turn off load relay to prevent over-discharging or overload
    }

    if ( AcsValueF > MAX_SOLAR_CURRENT )
    {
      digitalWrite(LOAD_RELAY_PIN, HIGH); // Turn on solar relay 
      ledcWrite(ledChannel, 0);         // Set PWM to 0 to disable charging current
    }
    else
    {
      digitalWrite(LOAD_RELAY_PIN, LOW); // Turn off solar relay 
    }
    
  }

  // Runs all Blynk stuff
  Blynk.run(); 

  timer.run(); // Initiates BlynkTimer
  
}

void myTimer() 
{
  // This function describes what will happen with each timer tick
  // Send sensor readings to Blynk app
  Blynk.virtualWrite(V0, voltage1); // Virtual pin V0 for solar panel voltage
  Blynk.virtualWrite(V1, voltage2); // Virtual pin V1 for battery voltage
  Blynk.virtualWrite(V2, voltage3); // Virtual pin V2 for load voltage
  Blynk.virtualWrite(V3, AcsValueF); // Virtual pin V3 for solar current
  Blynk.virtualWrite(V4, AcsValueF1); // Virtual pin V4 for load current
  Blynk.virtualWrite(V5, temp1); // Virtual pin V5 for temperature

  
  
}

// the same Virtual Pin as your Terminal Widget
BLYNK_WRITE(V6)
{


  // Debug mode control via Serial commands
    String command = param.asStr();
    
    if (command == "debug" || command == "Debug" || digitalRead(2) == HIGH) {
      debugMode = !debugMode;
      if (debugMode) {
        terminal.println("Debug mode enabled.");
        terminal.println("System Protection Triggered!");
        terminal.println("Turning off relays and MOSFET...");
      } else {
        terminal.println("Debug mode disabled.");
      }
    } else if (command == "sensor" || command == "Sensor") {
      
      // Send sensor readings
      
      printSensorReadings(voltage1, voltage2, voltage3, AcsValueF, AcsValueF1, temp1);
      
      
    } 
    
    else if (command == "threshold" || command == "Threshold") {

       terminal.println("Enter new threshold values (Format: minBattery,maxBattery,maxSolarCurrent,maxLoadCurrent,maxTemperature):");

       String input = param.asStr();
       int pos = input.indexOf(',');
   if (pos != -1)
  {
    String minBatteryStr = input.substring(0, pos);
    input = input.substring(pos + 1);
    pos = input.indexOf(',');
    if (pos != -1)
    {
      String maxBatteryStr = input.substring(0, pos);
      input = input.substring(pos + 1);
      pos = input.indexOf(',');
      if (pos != -1)
      {
        String maxSolarCurrentStr = input.substring(0, pos);
        input = input.substring(pos + 1);
        pos = input.indexOf(',');
        if (pos != -1)
        {
          String maxLoadCurrentStr = input.substring(0, pos);
          input = input.substring(pos + 1);
          String maxTemperatureStr = input;
  
          // Convert strings to floats
          MIN_BATTERY_VOLTAGE = minBatteryStr.toFloat();
          MAX_BATTERY_VOLTAGE = maxBatteryStr.toFloat();
          MAX_SOLAR_CURRENT = maxSolarCurrentStr.toFloat();
          MAX_LOAD_CURRENT = maxLoadCurrentStr.toFloat();
          MAX_TEMPERATURE = maxTemperatureStr.toFloat();
  
          terminal.println("Threshold values updated.");
          return;
        }
      }
    }
   }
     
}
  

  

//  // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
//  if (String("Marco") == param.asStr()) {
//       debugMode = !debugMode;
//        if (debugMode) {
//       terminal.println("Debug mode enabled.");
//      } else {
//       terminal.println("Debug mode disabled.");
//     }
//    terminal.println("You said: 'Marco'") ;
//    terminal.println("I said: 'Polo'") ;
//  } else {
//
//    // Send it back
//    terminal.print("You said:");
//    terminal.write(param.getBuffer(), param.getLength());
//    terminal.println();


  // Ensure everything is sent
  terminal.flush();
}


void printSensorReadings(float solarPanelVoltage, float batteryVoltage, float loadVoltage, float solarCurrent, float loadCurrent, float temperature) {
  
  terminal.println("Sensor Readings:");
  terminal.print("Solar Panel Voltage: "); terminal.print(solarPanelVoltage); terminal.println(" V");
  terminal.print("Battery Voltage: "); terminal.print(batteryVoltage); terminal.println(" V");
  terminal.print("Load Voltage: "); terminal.print(loadVoltage); terminal.println(" V");
  terminal.print("Solar Current: "); terminal.print(solarCurrent); terminal.println(" A");
  terminal.print("Load Current: "); terminal.print(loadCurrent); terminal.println(" A");
  terminal.print("Temperature: "); terminal.print(temperature); terminal.println(" °C");

  
}
