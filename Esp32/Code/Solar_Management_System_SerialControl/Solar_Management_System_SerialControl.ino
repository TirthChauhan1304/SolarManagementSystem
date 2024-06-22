#define SOLAR_RELAY_PIN 21 // Pin connected to the relay for solar side
#define LOAD_RELAY_PIN 22  // Pin connected to the relay for load side
#define MOSFET_PIN 23       // Pin connected to the gate of P-channel MOSFET

int Vsensor1, Vsensor2, Vsensor3, temp, X;
float voltage1, voltage2, voltage3, temp1,milliVolt;
float r1 = 47000.0;
float r2 = 33000.0;
unsigned int x = 0;
float AcsValue = 0.0, Samples = 0.0, AvgAcs = 0.0, AcsValueF = 0.0;
float AcsValue1 = 0.0, Samples1 = 0.0, AvgAcs1 = 0.0, AcsValueF1 = 0.0;

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

  Serial.begin(9600);
  Serial.println(" Solar Management System V 1.0.0 ");


}

void loop()
{
  
  Vsensor1 = analogRead(34); //Solar Pannel
  Vsensor2 = analogRead(35); //Battery
  Vsensor3 = analogRead(32); //Dc Load
  temp = analogRead(33);     //Temprature Sensor

  voltage1 = Vsensor1 * (3.30 / 4096.0) * ((r1 + r2) / r2);
  voltage2 = Vsensor2 * (3.30 / 4096.0) * ((r1 + r2) / r2);
  voltage3 = Vsensor3 * (3.30 / 4096.0) * ((r1 + r2) / r2);

  for (int x = 0; x < 150; x++)
  {                                   //Get 150 samples
    AcsValue = analogRead(25);        //Read current sensor values
    AcsValue1 = analogRead(26);       //Read current sensor values
    Samples = Samples + AcsValue;
    Samples1 = Samples1 + AcsValue1;  //Add samples together
    delay(3);                         // let ADC settle before next sample 3ms
  }

  AvgAcs = Samples / 150.0;           //Taking Average of Samples
  AvgAcs1 = Samples1 / 150.0;         //Taking Average of Samples

  //((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
  //0.100v(100mV) is rise in output voltage when 1A current flows at input

  AcsValueF = (2.5 - (AvgAcs * (3.30 / 4096.0))) / 0.100;
  AcsValueF1 = (2.5 - (AvgAcs1 * (3.30 / 4096.0))) / 0.100;

  milliVolt = temp * (3300/4096);
  // convert the voltage to the temperature in °C
  temp1 = milliVolt / 10;

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

 // Debug mode control via Serial commands
 
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == 'd' || command == 'D' || digitalRead(2) == HIGH) {
      debugMode = !debugMode;
      if (debugMode) {
        Serial.println("Debug mode enabled.");
      } else {
        Serial.println("Debug mode disabled.");
      }
    } else if (command == 's' || command == 'S') {
      
      // Send sensor readings
      printSensorReadings(voltage1, voltage2, voltage3, AcsValueF, AcsValueF1, temp1);
      
      
    } 
    else if (command == 't' || command == 'T') {
      // Change threshold values
      changeThresholdValues();
     
    }
  }

    Samples = 0;
    Samples1 = 0;

}

void printSensorReadings(float solarPanelVoltage, float batteryVoltage, float loadVoltage, float solarCurrent, float loadCurrent, float temperature) {
  
  Serial.println("Sensor Readings:");
  Serial.print("Solar Panel Voltage: "); Serial.print(solarPanelVoltage); Serial.println(" V");
  Serial.print("Battery Voltage: "); Serial.print(batteryVoltage); Serial.println(" V");
  Serial.print("Load Voltage: "); Serial.print(loadVoltage); Serial.println(" V");
  Serial.print("Solar Current: "); Serial.print(solarCurrent); Serial.println(" A");
  Serial.print("Load Current: "); Serial.print(loadCurrent); Serial.println(" A");
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" °C");

  
}

void changeThresholdValues()
{

  Serial.println("Enter new threshold values (Format: minBattery,maxBattery,maxSolarCurrent,maxLoadCurrent,maxTemperature):");

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
}
