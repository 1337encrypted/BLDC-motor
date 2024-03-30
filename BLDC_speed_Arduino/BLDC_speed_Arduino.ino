#include "GLOBALS.hpp"

void setup() {
  Serial.begin(115200);

  OLEDFunctions::begin(screenRenderTask, app_cpu1);

  serialBT.begin("Pod1_Bluetooth");

  serial.begin(uartHandle, app_cpu1);
  
  motorPulse1.begin(leftSpeedTaskHandle, app_cpu0);
  motorPulse2.begin(rightSpeedTaskHandle, app_cpu1);
  
  motorPWM1.begin(leftFrontTaskHandle, app_cpu0);
  motorPWM2.begin(rightFrontTaskHandle, app_cpu1); 

  direction.begin();

  sendDataToPhone.begin(dataLoggerTaskHandler, app_cpu0);
  
  controlMotor.begin(modifyMotorStateTaskHandle, app_cpu1);

  // vTaskDelay(100 / portTICK_PERIOD_MS);

  vTaskDelete(NULL);
}

void loop() {
  
}