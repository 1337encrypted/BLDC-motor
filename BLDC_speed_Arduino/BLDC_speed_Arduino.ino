#include <BluetoothSerial.h>
#include "GLOBALS.hpp"

BluetoothSerial SerialBT;

enum class States : uint8_t 
{
  FRONT,
  BACK,
  STOP,
  SPEED1,
  SPEED2,
  SPEED3,
  SPEED4,
  SPEED5,
  SPEED6,
  SPEED7,
  SPEED8,
  SPEED9,
  SPEED10,
};

States status = States::STOP;

States charToStates(char state) {
  switch (state) {
    case 'F': return States::FRONT;
    case 'B': return States::BACK;
    case 'S': return States::STOP;
    case '0': return States::SPEED1;
    case '1': return States::SPEED2;
    case '2': return States::SPEED3;
    case '3': return States::SPEED4;
    case '4': return States::SPEED5;
    case '5': return States::SPEED6;
    case '6': return States::SPEED7;
    case '7': return States::SPEED8;
    case '8': return States::SPEED9;
    case '9': return States::SPEED10;
  }
}

void setup() {
  Serial.begin(115200);

  SerialBT.begin("Pod1_Bluetooth");

  serial.begin(app_cpu1);
  
  PMSMMotor1.begin(app_cpu0);
  motor1.begin(app_cpu0);

  PMSMMotor2.begin(app_cpu1);
  motor2.begin(app_cpu1); 

  direction.begin();

  OLEDFunctions::begin(app_cpu1);

  vTaskDelay(100 / portTICK_PERIOD_MS);

  // vTaskDelete(NULL);
}

void loop() {
  if (SerialBT.available()) 
  {
    status = charToStates(SerialBT.read());
    snprintf(data, sizeof(data), ",%s,%s,%s,%s,%s", OLEDFunctions::oledSpeed1, OLEDFunctions::oledSpeed2, OLEDFunctions::currentArr1, OLEDFunctions::currentArr2, OLEDFunctions::voltageArr1);
    SerialBT.println(data);
  }
  else 
  {
    status = States::STOP;
  }

  switch (status) 
  {
    case States::FRONT:
      direction.front();
    break;

    case States::BACK:
      direction.back();
    break;
    
    case States::STOP:
      motor1.setPwm(0);
      motor2.setPwm(0);
    break;

    case States::SPEED1:
      motor1.setPwm(112);
      motor2.setPwm(112);
      break;
      
    case States::SPEED2:
      motor1.setPwm(126);
      motor2.setPwm(126);
      break;
      
    case States::SPEED3:
      motor1.setPwm(140);
      motor2.setPwm(140);
      break;

    case States::SPEED4:
      motor1.setPwm(154);
      motor2.setPwm(154);
      break;
    case States::SPEED5:
      motor1.setPwm(168);
      motor2.setPwm(168);
      break;

    case States::SPEED6:
      motor1.setPwm(182);
      motor2.setPwm(182);
      break;

    case States::SPEED7:
      motor1.setPwm(196);
      motor2.setPwm(196);
      break;

    case States::SPEED8:
      motor1.setPwm(210);
      motor2.setPwm(210);
      break;

    case States::SPEED9:
      motor1.setPwm(224);
      motor2.setPwm(224);
      break;

    case States::SPEED10:
      motor1.setPwm(255);
      motor2.setPwm(255);
      break;
  }
}


