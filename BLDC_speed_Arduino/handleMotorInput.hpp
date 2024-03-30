#pragma once
#include <BluetoothSerial.h>
// #include "BLDCPulseCalculator.hpp"
#include "PwmGenerator.hpp"
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
        SPEED6,
        SPEED7,
        SPEED8,
        SPEED9,
        SPEED10,
        SPEEDINC,
        SPEEDDEC,
        IDLE
    };
    States status;

    // BLDCPulseCalculator& motorPulse1;
    // BLDCPulseCalculator& motorPulse2;
    PwmGenerator& motorPWM1;
    PwmGenerator& motorPWM2;
    MotorDirection& direction;
    BluetoothSerial& serialBT;

    uint8_t leftDutyCycle, rightDutyCycle;

    public:
    handleMotorInput(/*BLDCPulseCalculator& ,BLDCPulseCalculator& ,*/PwmGenerator&, PwmGenerator&, MotorDirection&, BluetoothSerial& );

    void begin(TaskHandle_t &, const BaseType_t = 1);

    inline void setSpeed(uint8_t,uint8_t) __attribute__((always_inline));
    inline void modifyMotorState() __attribute__((always_inline));
    static inline void modifyMotorStateTask(void *pvParameters);

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
        case '6': return States::SPEED6;
        case '7': return States::SPEED7;
        case '8': return States::SPEED8;
        case '9': return States::SPEED9;
        case 'q': return States::SPEED10;
        case 'I': return States::SPEEDINC;
        case 'D': return States::SPEEDDEC;
        default: return States::IDLE; // or any appropriate default state
      }
    }
    
  };

  handleMotorInput::handleMotorInput (
  // BLDCPulseCalculator& motorPulse1,
  // BLDCPulseCalculator& motorPulse2,
  PwmGenerator& motorPWM1,
  PwmGenerator& motorPWM2,
  MotorDirection& direction,
  BluetoothSerial& serialBT) :
  // motorPulse1(motorPulse1),
  // motorPulse2(motorPulse2),
  motorPWM1(motorPWM1),
  motorPWM2(motorPWM2),
  direction(direction),
  serialBT(serialBT),
  status(States::IDLE),
  leftDutyCycle(0),
  rightDutyCycle(0)
  {}

  void handleMotorInput::setSpeed(uint8_t leftDutyCycle, uint8_t rightDutyCycle) {
    motorPWM1.setPwm(leftDutyCycle);
    motorPWM2.setPwm(rightDutyCycle);
  }

  void handleMotorInput::begin(TaskHandle_t &taskHandle, BaseType_t app_cpu) {

    const char* TAG = "handleMotorInput::begin";

  if(taskHandle == nullptr) {
    BaseType_t result = xTaskCreatePinnedToCore(
      &modifyMotorStateTask,
      "modifyMotorState",
      STACK_SIZE,
      this,
      1,
      &taskHandle,
      app_cpu
    );
    
    if (result == pdPASS){
          ESP_LOGI(TAG, "Created the modifyMotorStateTask successfully");
      } else {
          ESP_LOGI(TAG, "Failed to create the modifyMotorStateTask task");
      }
    } else {
      ESP_LOGI(TAG, "modifyMotorStateTask already created");
    }
  }

  void handleMotorInput::modifyMotorState() {
    if(serialBT.connected()) {
        if (serialBT.available()) {
          status = charToStates(serialBT.read());
        } 
      } else {
        status = States::STOP;
      }

    switch (status) {
      case States::IDLE :
      break;

      case States::SELFLEARNING:
        setSpeed(0,0);
        direction.selfLearning();
      break;

      case States::STOPSELFLEARNING:
        leftDutyCycle = rightDutyCycle = 0;
        setSpeed(0,0);
        direction.stopSelfLearning();
        status = States::IDLE;
      break;
      
      case States::STOP:
        setSpeed(0,0);
        leftDutyCycle = rightDutyCycle = 0;
        status = States::IDLE;
      break;

      case States::SPEED1:
        setSpeed(110,110);
        status = States::IDLE;
      break;
        
      case States::SPEED2:
        setSpeed(126,126);
        status = States::IDLE;
      break;
        
      case States::SPEED3:
        setSpeed(142,142);
        status = States::IDLE;
      break;

      case States::SPEED4:
        setSpeed(158,158);
        status = States::IDLE;
      break;

      case States::SPEED5:
        setSpeed(174,174);
        status = States::IDLE;
      break;

      case States::SPEED6:
        setSpeed(190,190);
        status = States::IDLE;
      break;
        
      case States::SPEED7:
        setSpeed(206,206);
        status = States::IDLE;
      break;
        
      case States::SPEED8:
        setSpeed(222,222);
        status = States::IDLE;
      break;

      case States::SPEED9:
        setSpeed(238,238);
        status = States::IDLE;
      break;

      case States::SPEED10:
        setSpeed(255,255);
        status = States::IDLE;
      break;

      case States::SPEEDINC:
        if(leftDutyCycle < 255)
          leftDutyCycle++;
        
        if(rightDutyCycle < 255)
          rightDutyCycle++;
        
        setSpeed(leftDutyCycle,rightDutyCycle);

        status = States::IDLE;
        vTaskDelay(50 / portTICK_PERIOD_MS);
      break;

      case States::SPEEDDEC:
        if(leftDutyCycle > 0)
          leftDutyCycle--;

        if(rightDutyCycle > 0)
          rightDutyCycle--;

      setSpeed(leftDutyCycle,rightDutyCycle);

        status = States::IDLE;
        vTaskDelay(50 / portTICK_PERIOD_MS);
      break;

      default:
      // Handle unexpected input or state
      status = States::IDLE;
      break;
    }
  }

  void handleMotorInput::modifyMotorStateTask(void *pvParameters) {
    handleMotorInput *instance = static_cast<handleMotorInput*>(pvParameters);
    while(true) {
      instance->modifyMotorState();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }