#include "GLOBALS.hpp"

void setup() {
  Serial.begin(115200);

  serial.begin(app_cpu1);
  
  PMSMMotor1.begin(app_cpu0);
  motor1.begin(app_cpu0);

  PMSMMotor2.begin(app_cpu1);
  motor2.begin(app_cpu1); 


  OLEDFunctions::begin(app_cpu1);

  vTaskDelay(100 / portTICK_PERIOD_MS);

  vTaskDelete(NULL);
}

void loop() {
  // Code never reaches here
}



