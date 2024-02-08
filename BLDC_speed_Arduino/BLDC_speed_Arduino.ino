#include "GLOBALS.hpp"

void setup() {
  Serial.begin(115200);

  PMSMMotor1.begin(app_cpu0);
  PMSMMotor2.begin(app_cpu1);

  // motorSynchronization()

  OLEDFunctions::begin();

  vTaskDelete(NULL);
}

void loop() {
  // Code never reaches here
}



