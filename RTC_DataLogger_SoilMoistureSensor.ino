// https://create.arduino.cc/editor/LogMaker360/9a5b5c2b-e510-4012-8a4c-70fbb17032b5/preview //
//#include <OneWire.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
const int chipSelect = 10; //cs or the save select pin from the sd shield is connected to 10.
RTC_DS1307 RTC;
//float celsius, fahrenheit;

//OneWire  ds(8);  // temperature senor on pin 8 (a 4.7K resistor is necessary) //
float reading = 0;
float val = 0;

File dataFile;
DateTime now;

void setup(void) {
  Serial.begin(9600);
  //setup clock
  Wire.begin();
  RTC.begin();
//check or the Real Time Clock is on
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // uncomment it & upload to set the time, date and start run the RTC!
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
//setup SD card
   Serial.print("Initializing SD card...");

  // see if the SD card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  //write down the date (year / month / day         prints only the start, so if the logger runs for sevenal days you only findt the start back at the begin.
    now = RTC.now();
    dataFile = SD.open("datalog.txt", FILE_WRITE);
    dataFile.print("Start logging on: ");
    dataFile.print(now.year(),DEC);
    dataFile.print('/');
    dataFile.print(now.month(),DEC);
    dataFile.print('/');
    dataFile.print(now.day(),DEC);
    dataFile.println(" ");
    dataFile.println("VMC%              Time");
    dataFile.close();
}

void loop(void) {
  
// read temperature
//pickUpTemperature();
//read the time
  now = RTC.now();
  
  //open file to log data in.
   dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  // log the temperature and time.
  if (dataFile) {
    
     // make a string for assembling the data to log:
      String dataString = "";

     // read three sensors and append to the string:
     for (int analogPin = 0; analogPin < 1; analogPin++) {
        val = analogRead(analogPin);
        val = 1023-val;
       reading = ((0.343642)*(val-747))+100; // (((MaxNew-MinNew)/(MaxOld-MinOld))*(Value-MaxOld))+MaxNew here MinNew=0, MaxNew=100, MaxOld=723, MinOld=431 i.e after subtracting from 1000
       dataString += String(reading);
       if (analogPin < 0) {
         dataString += ","; 
       }
     }

      // dataFile.println(dataString);

      // print to the serial port too:
      // Serial.println(dataString);
    dataFile.print(reading);
    dataFile.print("                 ");
    
    dataFile.print(now.hour(),DEC);
    dataFile.print(":");
    dataFile.print(now.minute(),DEC);
    dataFile.print(":");
    dataFile.println(now.second(),DEC);
   
    dataFile.close();
    // print to the serial port too:
    Serial.println("data stored");
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  //delay(60000); // this will log the temperature every minute.
  delay(5000);
}
