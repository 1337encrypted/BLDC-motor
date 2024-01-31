#include <stdio.h>
#include "BLDCPulseCalculator.hpp"

// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif


// Motor1 
constexpr gpio_num_t wavePin1 = GPIO_NUM_2;
constexpr uint8_t motorId1 = 1;

/* Object creation */
BLDCPulseCalculator PMSMMotor1(wavePin1, motorId1);


extern "C"
{
  void app_main(void)
  {
    PMSMMotor1.begin(app_cpu);
  }
}