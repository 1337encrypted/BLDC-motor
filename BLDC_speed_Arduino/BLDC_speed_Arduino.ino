#include "GLOBALS.hpp"

void setup() {
  Serial.begin(115200);

  OLEDFunctions::begin(app_cpu1);

  motorPulse1.begin(app_cpu0);
  motorPulse2.begin(app_cpu1);
  motorPWM1.begin(app_cpu1);
  motorPWM2.begin(app_cpu1); 
  direction.begin();
  
  serial.begin(app_cpu1);
  SerialBT.begin("Pod1_Bluetooth");
  sendDataToPhone.begin(app_cpu0);
  
  controlMotor.begin(app_cpu1);


  // vTaskDelay(100 / portTICK_PERIOD_MS);
  vTaskDelete(NULL);
}

void loop() {
}