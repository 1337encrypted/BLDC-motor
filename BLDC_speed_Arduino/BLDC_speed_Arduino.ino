#include "GLOBALS.hpp"

void setup() {
  Serial.begin(115200);

  OLEDFunctions::begin(app_cpu1);

  SerialBT.begin("Pod1_Bluetooth");

  serial.begin(app_cpu1);
  
  PMSMMotor1.begin(app_cpu0);
  motor1.begin(app_cpu0);

  PMSMMotor2.begin(app_cpu1);
  motor2.begin(app_cpu1); 

  direction.begin();

  sendDataToPhone.begin(app_cpu0);
  
  controlMotor.begin(app_cpu1);

  // vTaskDelay(100 / portTICK_PERIOD_MS);

  // vTaskDelete(NULL);
}

void loop() {
  
}