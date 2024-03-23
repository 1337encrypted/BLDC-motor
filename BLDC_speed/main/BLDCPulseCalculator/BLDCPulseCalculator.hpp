#pragma once

#include <driver/gpio.h>
#include <esp_log.h>
#include <cstring>
#include "esp_timer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
// #include "oledFunctions.hpp"

class BLDCPulseCalculator {
private:
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  gpio_num_t wavePin;
  uint8_t motorId;

  volatile uint8_t counter;
  volatile unsigned long nextTimeStamp;
  volatile unsigned long timeStamp;

  volatile uint32_t timePeriodValues[32];
  volatile uint32_t newTimePeriodValues[16];

  volatile uint16_t speed;


  enum class BLDCstates : uint8_t {
    IDLE,
    PRINT
  };
  volatile BLDCstates status = BLDCstates::IDLE;

  // static BLDCPulseCalculator* instance;
  

public:



  BLDCPulseCalculator(gpio_num_t wavePin = GPIO_NUM_NC, uint8_t motorId = -1);
  void begin(const BaseType_t = 1);
  
  inline void calculateValuesInternal(void) __attribute__((always_inline));
  inline void motorSpeed() __attribute__((always_inline));
  
  static inline void motorSpeedTask(void*) __attribute__((always_inline));

  // Interrupt containing functions cannot be inlined
  static void staticCalculateValuesWrapper(void *);
};