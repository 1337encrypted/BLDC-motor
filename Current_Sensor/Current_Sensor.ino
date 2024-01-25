// Use only core 1 for demo purposes
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

#include "GLOBALS.h"



void setup()
{ 
  Serial.begin(115200);

  vTaskDelay(1000 / portTICK_PERIOD_MS);


  // pulseCalculator1.begin();
  // pulseCalculator2.begin();

  ACS712Sensor.begin();

  // Delete "setup and loop" task
  // vTaskDelete(NULL);
}

void loop()
{
  //Code should not enter here
  ACS712Sensor.printCurrent();

  // vTaskDelay(500 / portTICK_PERIOD_MS);
}

