#include "HX711.h"

HX711 cell(3, 2);

void setup() {
  Serial.begin(9600);
}

long val = 0;
float count = 0;

void loop() {
  //count = count + 1;
  
  // Use only one of these
  //val = ((count-1)/count) * val    +  (1/count) * cell.read(); // take long term average
  //val = 0.5 * val    +   0.5 * cell.read(); // take recent average
  val = cell.read(); // most recent reading
  // zero =8427045
  //Serial.println(val);  
  Serial.println((val-8427701)/-19302.f * 180);
  delay(5000);
}
