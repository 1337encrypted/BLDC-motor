#include "GLOBALS.h"

void setup()
{ 
  Serial.begin(115200);
  delay(1000);
  OLEDFunctions::begin();
  ACS712Sensor.begin();

  memset(currentArr, 0, sizeof(currentArr));
  // memset(voltageArr, 0, sizeof(voltageArr));
  // memset(powerArr, 0, sizeof(powerArr));
}

void loop()
{
  dtostrf(ACS712Sensor.calculateCurrent(), 5, 2, currentArr);
  OLEDFunctions::displayCurrent(currentArr);
}

