#include <stdio.h>
#include "BLDCPulseCalculator.hpp"
#include "oledFunctions.hpp"
// #include "motorSynchronization.hpp"
#include "pwmGenerator.hpp"
#include <U8g2lib.h>


// Use only core 1 for demo purposes
// #if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu0 = 0;
// #else
  static const BaseType_t app_cpu1 = 1;
// #endif

// Motor1 
constexpr gpio_num_t wavePin1 = GPIO_NUM_2;
constexpr gpio_num_t wavePin2 = GPIO_NUM_15;

//MotorSynchronization
constexpr gpio_num_t motorPwm1 = GPIO_NUM_12;
constexpr gpio_num_t motorPwm2 = GPIO_NUM_14; 

const uint32_t frequency = 100000;           // PWM frequency in Hz
const uint8_t resolution = 8;        // PWM resolution (8-bit)

constexpr uint8_t motorId1 = 1;
constexpr uint8_t motorId2 = 2;

/* Object creation */
BLDCPulseCalculator PMSMMotor1(wavePin1, motorId1);
BLDCPulseCalculator PMSMMotor2(wavePin2, motorId2);

// motorSynchronization motor1(motorPwm1);
PwmGenerator motor1(motorPwm1, frequency, resolution);
PwmGenerator motor2(motorPwm2, frequency, resolution);