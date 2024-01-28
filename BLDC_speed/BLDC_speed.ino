// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#include "BLDCPulseCalculator.h"

// Motor1 
constexpr int8_t wavePin1 = 2;
constexpr uint8_t motorId1 = 1;

/* Object creation */
BLDCPulseCalculator PMSMMotor1(wavePin1, motorId1);

void setup()
{
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  PMSMMotor1.begin(app_cpu);

  //Delete the setup and loop
  vTaskDelete(NULL);
}

void loop()
{
  //This gets deleted
}