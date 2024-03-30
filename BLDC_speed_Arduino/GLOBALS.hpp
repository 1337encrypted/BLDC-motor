#include <stdio.h>
#include <BluetoothSerial.h>
#include "uart.hpp"
#include "BLDCPulseCalculator.hpp"
#include "oledFunctions.hpp"
#include "PwmGenerator.hpp"
#include "MotorDirection.hpp"
#include "dataLogger.hpp"
#include "handleMotorInput.hpp"

// Use only core 1 for demo purposes
// #if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu0 = 0;
// #else
  static const BaseType_t app_cpu1 = 1;
// #endif

//Motor Properties
const uint32_t frequency = 100000;                            // PWM frequency in Hz
const ledc_timer_bit_t resolution = LEDC_TIMER_8_BIT;         // PWM resolution (8-bit)

// Motor1 
constexpr uint8_t motorId1 = 1;
constexpr gpio_num_t feedBackPin1 = GPIO_NUM_2;
constexpr gpio_num_t motorPwmPin1 = GPIO_NUM_12;
constexpr gpio_num_t triggerPin1 = GPIO_NUM_19;

// Motor2
constexpr uint8_t motorId2 = 2;
constexpr gpio_num_t feedBackPin2 = GPIO_NUM_15;
constexpr gpio_num_t motorPwmPin2 = GPIO_NUM_14; 
constexpr gpio_num_t triggerPin2 = GPIO_NUM_23;

/* Object creation */
BLDCPulseCalculator motorPulse1(feedBackPin1, motorId1);
BLDCPulseCalculator motorPulse2(feedBackPin2, motorId2);

// motorSynchronization motor1(motorPwm1);
PwmGenerator motorPWM1(motorPwmPin1, frequency, resolution);
PwmGenerator motorPWM2(motorPwmPin2, frequency, resolution);

// Uart object
HardwareUart serial;

// Motor direction
MotorDirection direction(triggerPin1,triggerPin2);

// Bluetooth object
BluetoothSerial serialBT;

// Data logger
dataLogger sendDataToPhone(serialBT);

// MotorSynchrnization object
// handleMotorInput controlMotor(motorPulse1,motorPulse2,motorPWM1,motorPWM2,direction,SerialBT);
handleMotorInput controlMotor(motorPWM1,motorPWM2,direction,serialBT);


// Task handles
TaskHandle_t screenRenderTask = nullptr;
TaskHandle_t uartHandle = nullptr;
TaskHandle_t leftSpeedTaskHandle = nullptr;
TaskHandle_t rightSpeedTaskHandle = nullptr;
TaskHandle_t leftFrontTaskHandle = nullptr;
TaskHandle_t rightFrontTaskHandle = nullptr;
TaskHandle_t dataLoggerTaskHandler = nullptr;
TaskHandle_t modifyMotorStateTaskHandle = nullptr;
