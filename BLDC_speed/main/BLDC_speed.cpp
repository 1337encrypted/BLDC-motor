#include "GLOBALS.hpp"

extern "C"
{
  void app_main(void)
  {
    serial.begin(app_cpu1);
  
    PMSMMotor1.begin(app_cpu0);
    PMSMMotor2.begin(app_cpu1);

    // OLEDFunctions::begin(app_cpu1);

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



