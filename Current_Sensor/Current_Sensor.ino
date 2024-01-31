#include "GLOBALS.hpp"

void setup()
{ 
  Serial.begin(115200);
  delay(1000);
  OLEDFunctions::begin();
  ammeter1.begin();
  voltMeter.begin();

  memset(currentArr, 0, sizeof(currentArr));
  memset(voltageArr, 0, sizeof(voltageArr));
  // memset(powerArr, 0, sizeof(powerArr));
}

void loop()
{
  dtostrf(ammeter1.readCurrent(), 5, 2, currentArr);
  dtostrf(voltMeter.readVoltage(), 5, 2, voltageArr);


  OLEDFunctions::displayCurrent(currentArr);
  OLEDFunctions::displayVoltage(voltageArr);
  // OLEDFunctions::displayPower(powerArr);
}

