#pragma once
#include <BluetoothSerial.h>
// #include "BLDCPulseCalculator.hpp"
#include "pwmGenerator.hpp"
#include "MotorDirection.hpp"
#include "dataLogger.hpp"

class handleMotorInput {
private:
  enum class States : uint8_t {
      SELFLEARNING,
      STOPSELFLEARNING,
      STOP,
      SPEED1,
      SPEED2,
      SPEED3,
      SPEED4,
      SPEED5,
      SPEEDINC,
      SPEEDDEC,
      IDLE
  };
  States status;
  States prevStatus;

  // BLDCPulseCalculator& motorPulse1;
  // BLDCPulseCalculator& motorPulse2;
  PwmGenerator& motorPWM1;
  PwmGenerator& motorPWM2;
  MotorDirection& direction;
  BluetoothSerial& SerialBT;

  uint8_t leftDutyCycle, rightDutyCycle;

  // Task handle
  TaskHandle_t modifyMotorStateHandle;

public:
  handleMotorInput(
    // BLDCPulseCalculator& motorPulse1,
    // BLDCPulseCalculator& motorPulse2,
    PwmGenerator& motorPWM1,
    PwmGenerator& motorPWM2,
    MotorDirection& direction,
    BluetoothSerial& SerialBT) :
    // motorPulse1(motorPulse1),
    // motorPulse2(motorPulse2),
    motorPWM1(motorPWM1),
    motorPWM2(motorPWM2),
    direction(direction),
    SerialBT(SerialBT),
    status(States::IDLE),
    prevStatus(status),
    leftDutyCycle(0),
    rightDutyCycle(0),
    modifyMotorStateHandle(nullptr) {
      // Constructor body if needed
  }

  void begin(BaseType_t app_cpu) {

    const char* TAG = "handleMotorInput::begin";

    if(modifyMotorStateHandle == nullptr) {
      BaseType_t result = xTaskCreatePinnedToCore(
          &modifyMotorStateTask,
          "modifyMotorState",
          STACK_SIZE,
          this,
          1,
          &modifyMotorStateHandle,
          app_cpu
      );

      if (result == pdPASS){
          ESP_LOGI(TAG, "Created the UartTask successfully");
      } else {
          ESP_LOGI(TAG, "Failed to create the UartTask task");
      }
    } else {
        ESP_LOGI(TAG, "UartTask already created");
    }
  }


  States charToStates(char state) {
    switch (state) {
      case 'M': return States::SELFLEARNING;
      case 'X': return States::STOPSELFLEARNING;
      case 'S': return States::STOP;
      case '1': return States::SPEED1;
      case '2': return States::SPEED2;
      case '3': return States::SPEED3;
      case '4': return States::SPEED4;
      case '5': return States::SPEED5;
      case 'I': return States::SPEEDINC;
      case 'D': return States::SPEEDDEC;
      default: return States::IDLE; // or any appropriate default state
    }
  }

  void modifyMotorState() {
    if(! SerialBT.isClosed() && SerialBT.connected()) {
      if (SerialBT.available()) {
        status = charToStates(SerialBT.read());
      } 
    } else {
      status = States::STOP;
    }

    switch (status) {
      case States::IDLE :
      break;

      case States::SELFLEARNING:
        direction.selfLearning();
      break;

      case States::STOPSELFLEARNING:
        direction.stopSelfLearning();
        status = States::IDLE;
      break;
      
      case States::STOP:
        motorPWM1.setPwm(0);
        motorPWM2.setPwm(0);
        leftDutyCycle = rightDutyCycle = 0;
        status = States::IDLE;
      break;

      case States::SPEED1:
        motorPWM1.setPwm(112);
        motorPWM2.setPwm(112);
        status = States::IDLE;
      break;
        
      case States::SPEED2:
        motorPWM1.setPwm(126);
        motorPWM2.setPwm(126);
        status = States::IDLE;
      break;
        
      case States::SPEED3:
        motorPWM1.setPwm(140);
        motorPWM2.setPwm(140);
        status = States::IDLE;
      break;

      case States::SPEED4:
        motorPWM1.setPwm(154);
        motorPWM2.setPwm(154);
        status = States::IDLE;
      break;

      case States::SPEED5:
        motorPWM1.setPwm(168);
        motorPWM2.setPwm(168);
        status = States::IDLE;
      break;

      case States::SPEEDINC:
        if(leftDutyCycle < 255)
          leftDutyCycle++;
        
        if(rightDutyCycle < 255)
          rightDutyCycle++;
        
        motorPWM1.setPwm(leftDutyCycle);
        motorPWM2.setPwm(rightDutyCycle);

        status = States::IDLE;
        vTaskDelay(50 / portTICK_PERIOD_MS);
      break;

      case States::SPEEDDEC:
        if(leftDutyCycle > 0)
          leftDutyCycle--;

        if(rightDutyCycle > 0)
          rightDutyCycle--;

        motorPWM1.setPwm(leftDutyCycle);
        motorPWM2.setPwm(rightDutyCycle);

        status = States::IDLE;
        vTaskDelay(50 / portTICK_PERIOD_MS);
      break;

      default:
      // Handle unexpected input or state
      status = States::IDLE;
      break;
    }
  }

  static void modifyMotorStateTask(void *pvParameters) {
    handleMotorInput *instance = static_cast<handleMotorInput*>(pvParameters);
    while(true) {
      instance->modifyMotorState();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }
};