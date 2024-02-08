#include <string.h>
#include "GLOBALS.hpp"

void setup()
{ 
  Serial.begin(115200);
  delay(100);

  OLEDFunctions::begin();

  ammeter1.begin();
  ammeter2.begin();
  // voltMeter1.begin();
  // voltMeter2.begin();
  buzz.begin();

  memset(currentArr1, 0, sizeof(currentArr1));
  memset(currentArr2, 0, sizeof(currentArr2));
  // memset(voltageArr1, 0, sizeof(voltageArr1));
  // memset(voltageArr2, 0, sizeof(voltageArr2));
  // memset(powerArr1, 0, sizeof(powerArr1));
  // memset(powerArr2, 0, sizeof(powerArr2));

  buzz.initBuzzer();
}

void loop()
{
  current1 = ammeter1.readCurrent();
  current2 = ammeter2.readCurrent();

  // voltage1 = voltMeter1.readVoltage();
  // voltage2 = voltMeter2.readVoltage();

  // power1 = current1*voltage1;
  // power2 = current2*voltage2;

  dtostrf(current1, 4, 2, currentArr1);
  dtostrf(current2, 4, 2, currentArr2);

  // dtostrf(voltage1, 4, 2, voltageArr1);
  // dtostrf(voltage2, 4, 2, voltageArr2);

  // dtostrf(power1, 4, 2, powerArr1);
  // dtostrf(power2, 4, 2, powerArr2); 

  if(current1 > 8.0 || current2 > 8.0)
    buzz.on();
  else
    buzz.off();

  OLEDFunctions::clearBuffer();

  OLEDFunctions::displayCurrent(currentArr1, currentArr2);
  // OLEDFunctions::displayVoltage(voltageArr1);
  // OLEDFunctions::displayPower(powerArr1, powerArr2);

  OLEDFunctions::sendBuffer();

  // printValues(power1, power2);
  
  delay(2000);
}

void printValues(double power1, double power2){
  ammeter1.printCurrent();
  ammeter2.printCurrent();
  // voltMeter1.printVoltage();
  // voltMeter2.printVoltage();
  // Serial.println(power1);
  // Serial.println(power2);
  Serial.println();
}
