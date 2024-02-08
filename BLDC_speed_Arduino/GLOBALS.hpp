#include <stdio.h>
#include "BLDCPulseCalculator.hpp"
#include <U8g2lib.h>
#include "oledFunctions.hpp"

// Use only core 1 for demo purposes
// #if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu0 = 0;
// #else
  static const BaseType_t app_cpu1 = 1;
// #endif

// Motor1 
constexpr gpio_num_t wavePin1 = GPIO_NUM_15;
constexpr gpio_num_t wavePin2 = GPIO_NUM_2;

constexpr uint8_t motorId1 = 1;
constexpr uint8_t motorId2 = 2;

/* Object creation */
BLDCPulseCalculator PMSMMotor1(wavePin1, motorId1);
BLDCPulseCalculator PMSMMotor2(wavePin2, motorId2);

// motorSynchronization driveSystem(&PMSMMotor1, &PMSMMotor2);