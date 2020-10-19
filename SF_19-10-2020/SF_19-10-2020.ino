// Arduino IDE version used is 1.8.13
// All libraries required
#include <Wire.h>
#include <Adafruit_INA219.h> /* Version 1.0.9 */
#include <Adafruit_ADS1015.h> /* ADS1X15 1.1.1 Version  */
// SD card and RTC libraries
#include <SPI.h>
#include <SD.h>
#include "RTClib.h" /* Adafruit RTClib.h 1.10.0 */
RTC_DS1307 RTC;
 
//Heating and Data Logging Intervals
#define INTERVAL_0 60000 /* For displaying current readings once in a minute. Enter the product of 1000 milli seconds * 60 seconds * 1 minute = 60000 milli seconds*/
#define INTERVAL_1 30000 /*Waiting 15 minutes, then enter the product of 1000 milli seconds * 60 seconds * 15 minute = 900000 milli seconds*/
#define INTERVAL_2 8000UL /*Heating 8 seconds, then enter the product of 1000 milli seconds * 8 seconds = 8000 milli seconds*/
#define INTERVAL_3 100 /*Thermistor temperature logging at 1/10th of a second*/


// Global variables
const int chipSelect = 10; //save select pin from the sd shield is connected to 10
int16_t adc0, adc1; /* Thermistor readings adc0 at base and adc1 downstream */
unsigned long currentMillis, previousMillis = 0;
unsigned long time_0 = 0; /* For current readings using INA219 */
unsigned long time_3 = 0; /* For thermistors reading */
// Library calls
Adafruit_INA219 ina219; /* Current drawn details, can be removed later */
Adafruit_ADS1115 ads;  /* For the 16-bit version */
File dataFile;
DateTime now;

// Setup
void setup(void) 
{
  pinMode(9, OUTPUT); /* Pin9 is the PWM pin for heating gauge */
  digitalWrite(9, LOW); /* Pin9 kept at LOW by default */
  Serial.begin(115200);
  // INA219 current and voltage measurement part
  ads.begin(); 
  uint32_t currentFrequency;
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  // to use a lower 16V, 400mA range (higher precision on volts and amps as per our power supply
  ina219.setCalibration_16V_400mA();
  Serial.println("Measuring voltage and current with INA219");
  
  //SD card and RTC part
  Wire.begin();
  RTC.begin();
  //check or the Real Time Clock is on
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    RTC.adjust(DateTime(__DATE__, __TIME__)); /* Sets the date and time to compilation date and time */
  }
  //setup SD card
   Serial.print("Initializing SD card...");

  // see if the SD card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return; /* halt the program */
  }
  Serial.println("card initialised.");
    now = RTC.now();
    dataFile = SD.open("datalog.txt", FILE_WRITE);
    dataFile.print("Start logging on: ");
    dataFile.print(now.year(),DEC);
    dataFile.print('/');
    dataFile.print(now.month(),DEC);
    dataFile.print('/');
    dataFile.print(now.day(),DEC);
    dataFile.println(" ");
    dataFile.println("base, dstream, time");
    dataFile.close();
  
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
  
  float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */
  adc0 = ads.readADC_SingleEnded(0);        // read single AIN0 near the heating gauge
  adc1 = ads.readADC_SingleEnded(1);        // read single AIN1 downstream in the stem
  float base = adc0 * multiplier;
  float dstream = adc1 * multiplier;
  
  if(millis() > time_3 + INTERVAL_3){
      time_3 = millis();
      Serial.print("A0: ");
      Serial.print(base); Serial.print("mV   ");
      Serial.println();
      Serial.print("A1: ");
      Serial.print(dstream); Serial.print("mV ");
      Serial.println();
      // Data Logging Part
      // Read the time from onboard RTC
      now = RTC.now();
  
      // Open file to log data in.
      dataFile = SD.open("datalog.txt", FILE_WRITE);
      if (dataFile) {
        dataFile.print(now.hour(),DEC);
        dataFile.print(":");
        dataFile.print(now.minute(),DEC);
        dataFile.print(":");
        dataFile.println(now.second(),DEC);
        dataFile.print(base);
        dataFile.print(",");
        dataFile.print(dstream);
        dataFile.print(",");
        dataFile.close();
        Serial.println(base);
        Serial.println(dstream);
        Serial.println("data stored");
        }
        // if the file isn't open, pop up an error
      else {
      Serial.println("error opening datalog.txt");
      }
      }   
}

//Sketch uses 20062 bytes (62%) of program storage space. Maximum is 32256 bytes.
//Global variables use 1503 bytes (73%) of dynamic memory, leaving 545 bytes for local variables. Maximum is 2048 bytes.
  
  
