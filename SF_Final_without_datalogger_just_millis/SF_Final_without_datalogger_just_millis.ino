//All libraries required
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_ADS1015.h>

//Heating and Data Logging Intervals
#define INTERVAL_0 1000*60 /* For displaying current readings once in a minute*/
#define INTERVAL_1 1000*60*15 /*Waiting 15 minutes*/
#define INTERVAL_2 8000 /*Heating 8 seconds*/
#define INTERVAL_3 100 /*Thermistor temperature logging at 1/10th of a second*/

// Global variables
int16_t adc0, adc1; /* Thermistor readings adc0 at base and adc1 downstream */
unsigned long currentMillis, previousMillis = 0;
unsigned long time_0 = 0; /* For current readings using INA219 */
unsigned long time_3 = 0; /* For thermistors reading */
// Library calls
Adafruit_INA219 ina219; /* Current drawn details, can be removed later */
Adafruit_ADS1115 ads;  /* For the 16-bit version */

// Setup
void setup(void) 
{
  pinMode(9, OUTPUT); /* Pin9 is the PWM pin for heating gauge */
  digitalWrite(9, LOW); /* Pin9 kept at LOW by default */
  Serial.begin(115200);
  ads.begin(); 
  uint32_t currentFrequency;
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  // to use a lower 16V, 400mA range (higher precision on volts and amps as per our power supply
  ina219.setCalibration_16V_400mA();
  Serial.println("Measuring voltage and current with INA219");
}

// Loop
void loop(void) 
{
  // INA 219 current measurement
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);
  if(millis() > time_0 + INTERVAL_0){
    time_0 = millis();
    Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
    Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
    Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
    Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
    Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
    Serial.println("");
  }
  
  // Heater gauge
  currentMillis = millis();
  // For heater gauge on for 8 seconds at 50% duty cycle
  if(currentMillis - previousMillis >= INTERVAL_1){
    analogWrite(9, 127); /* 127/226 is 50% duty cycle, change the PWM as per requirement */
  }
  // For heater gauge waiting duration 15 mins, use shorter cycles for testing the code
  if(currentMillis - previousMillis >= (INTERVAL_1 + INTERVAL_2)){
    digitalWrite(9, LOW); /* Heating gauge set to low*/
    previousMillis = currentMillis;/* Resetting the clock */
  }
  
  int16_t results;
  float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */
  adc0 = ads.readADC_SingleEnded(0);        // read single AIN0 near the heating gauge
  adc1 = ads.readADC_SingleEnded(1);        // read single AIN1 downstream in the stem
  if(millis() > time_3 + INTERVAL_3){
      time_3 = millis();
      Serial.print("A0: ");
      Serial.print(adc0 * multiplier); Serial.print("mV   ");
      Serial.println();
      Serial.print("A1: ");
      Serial.print(adc1 * multiplier); Serial.print("mV ");
      Serial.println();
      Serial.println("--------------------------------------");
  } 
}

  
  
