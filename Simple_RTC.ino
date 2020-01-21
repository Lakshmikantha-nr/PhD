// https://create.arduino.cc/editor/LogMaker360/9a5b5c2b-e510-4012-8a4c-70fbb17032b5/preview //
#include <OneWire.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
const int chipSelect = 10; //cs or the save select pin from the sd shield is connected to 10.
RTC_DS1307 RTC;
float celsius, fahrenheit;

OneWire  ds(8);  // temperature senor on pin 8 (a 4.7K resistor is necessary) //

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
    dataFile.println("Celsius              Time");
    dataFile.close();
}

void loop(void) {
  
// read temperature
pickUpTemperature();
//read the time
  now = RTC.now();
  
  //open file to log data in.
   dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  // log the temperature and time.
  if (dataFile) {
    dataFile.print(celsius);
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
  delay(3000);
}


// fuction with check the temperature sensor and update the tempeature. 
void pickUpTemperature(){
    byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
 
  
  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
   //Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  //Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
   // Serial.write(' ');
   // Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
     // Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
    //  Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
    //  Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
   //   Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  //Serial.print("  Data = ");
  //Serial.print(present, HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
   // Serial.print(data[i], HEX);
   // Serial.print(" ");
  }
 // Serial.print(" CRC=");
 // Serial.print(OneWire::crc8(data, 8), HEX);
 // Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  
}
