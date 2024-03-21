#include <stdio.h>
#include "GLOBALS.hpp"


// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

U8G2_SSD1306_128X64_NONAME_F_HW_I2C menuList(U8G2_R0);   

// Motor1 
constexpr gpio_num_t wavePin1 = GPIO_NUM_15;
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



