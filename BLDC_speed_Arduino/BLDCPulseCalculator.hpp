#pragma once

#include <driver/gpio.h>
#include <esp_log.h>
#include <cstring>
#include "esp_timer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "oledFunctions.hpp"

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



  inline BLDCPulseCalculator(gpio_num_t wavePin = GPIO_NUM_NC, uint8_t motorId = -1) __attribute__((always_inline));
  inline void calculateValuesInternal(void) __attribute__((always_inline));
  inline void motorSpeed() __attribute__((always_inline));
  inline uint16_t getSpeed() __attribute__((always_inline));
  
  inline void begin(const BaseType_t = 1) __attribute__((always_inline));
  static inline void motorSpeedTask(void*) __attribute__((always_inline));
  static void staticCalculateValuesWrapper(void *);
};

// BLDCPulseCalculator* BLDCPulseCalculator::instance = nullptr;

BLDCPulseCalculator::BLDCPulseCalculator(gpio_num_t wavePin, uint8_t motorId) :
wavePin(wavePin), 
motorId(motorId),
speed(0)
{
  memset((void *)(timePeriodValues), 0, sizeof(timePeriodValues));
  memset((void *)(newTimePeriodValues), 0, sizeof(newTimePeriodValues));

  this->counter=0;
  this->nextTimeStamp=0;
  this->timeStamp=0;
  // instance = this;
}

/* 
  ### Important concept !!!
  
  - The freeRTOS task creation function xTaskCreate or xTaskCreatePinnedToCore and interrupt handler gpio_isr_handler_add() creation functions
    require you to pass a static function as a parameter they do not accept non-static class memebers as parameters.

  - Therefore the wrapper functions for the tasks you create or even the interrupt functions that you create such as 

    void IRAM_ATTR BLDCPulseCalculator::staticCalculateValuesWrapper(void *args)
    void BLDCPulseCalculator::motorSpeedTask(void* pvParameters)

    should be declared as static but can be a part of the class by writing the prototype in the public access specifier, these methods do not have 
    access to any data variables and data methods.

  - Getting access to the data members and variables can either be done by creating a static instance like 
    
  static BLDCPulseCalculator* instance;                                 // in private access specifier
  
  BLDCPulseCalculator::BLDCPulseCalculator(){

    instance = this;     

  //This won't have global access so do not go down this path unless you want to use a method from a class to pass this instance to a static method
  }

  BLDCPulseCalculator* BLDCPulseCalculator::instance = nullptr;         // You can initilze it inside the constructor or outside the class
  
  // Initilise it outisde class defiinition as a nullptr, this will have access globally so all the static methods can benefit from this.

  OR

  - Do it like i did in this class, use the begin method to create your xtasks and the interrupt can also be created inside the method

  void BLDCPulseCalculator::begin(const BaseType_t app_cpu) {
    gpio_install_isr_service(0);
    gpio_isr_handler_add(this->wavePin, staticCalculateValuesWrapper, (void*)(this));

    xTaskCreatePinnedToCore(
      motorSpeedTask,
      "motorSpeedTask",
      2048,
      this,
      1,
      NULL,
      app_cpu
    );
  }

  Use the this keyword to pass the instance without creating any additional instatnces.

*/

void BLDCPulseCalculator::begin(const BaseType_t app_cpu) {

  gpio_config_t gpioOutputConfigure = {
    .pin_bit_mask = (1ULL << this->wavePin),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_ANYEDGE
  };
  ESP_ERROR_CHECK(gpio_config(&gpioOutputConfigure));

  // gpio_set_direction(wavePin, GPIO_MODE_INPUT);
  // gpio_pulldown_en(wavePin);
  // gpio_pullup_dis(wavePin);
  // gpio_set_intr_type(wavePin, GPIO_INTR_ANYEDGE);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(this->wavePin, staticCalculateValuesWrapper, (void*)(this));

  xTaskCreatePinnedToCore(
    motorSpeedTask,
    "motorSpeedTask",
    2048,
    this,
    1,
    NULL,
    app_cpu
  );
}

void BLDCPulseCalculator::calculateValuesInternal() {
  // static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  portENTER_CRITICAL_ISR(&mux);

  timeStamp = esp_timer_get_time() / 1000;  // Convert to milliseconds
  timePeriodValues[counter] = static_cast<uint32_t>(timeStamp - nextTimeStamp);
  counter = counter + 1;
  nextTimeStamp = timeStamp;

  if ((counter % 2) == 0 && counter != 0) {
    newTimePeriodValues[counter / 2 - 1] = timePeriodValues[counter - 1] + timePeriodValues[counter - 2];
  }

  if (counter == 32) {
    counter = 0;
    status = BLDCstates::PRINT;
  }

  portEXIT_CRITICAL_ISR(&mux);
}


void BLDCPulseCalculator::motorSpeed() {
  uint32_t sumTime = 0;

  switch (status) {
    case BLDCstates::PRINT:
      for (int i = 0; i < 16; i++) {
        sumTime += newTimePeriodValues[i];
      }

      // timePeriod = (uint32_t *)(sumTime / 16.0);
      // speed = (uint16_t)(3750 / timePeriod);

      speed = static_cast<uint16_t>(60000 / sumTime);
      // ESP_LOGI("MOTOR", "Speed %u", speed);
      // Serial.println(speed);

      // OLEDFunctions::displayRPM(speed, motorId);
      if(motorId == 1)
      {
        itoa(speed, OLEDFunctions::oledSpeed1, 10); // Using base 10
      }
      else
      {
        itoa(speed, OLEDFunctions::oledSpeed2, 10); // Using base 10
      }

      sumTime = 0;
      status = BLDCstates::IDLE;
      break;

    case BLDCstates::IDLE:
      // This is to set the speed to 0 when the wheel is not moving
      if(((esp_timer_get_time() / 1000) - nextTimeStamp) > 2000)
      {
        speed = 0;
        if(motorId == 1)
          itoa(speed, OLEDFunctions::oledSpeed1, 10); // Using base 10
        else
          itoa(speed, OLEDFunctions::oledSpeed2, 10); // Using base 10
      }
      break;
  }
}

void IRAM_ATTR BLDCPulseCalculator::staticCalculateValuesWrapper(void *args) {
  BLDCPulseCalculator* instance = static_cast<BLDCPulseCalculator*>(args);
  if (instance) {
    instance->calculateValuesInternal();
  }
}

void BLDCPulseCalculator::motorSpeedTask(void* pvParameters) {
  BLDCPulseCalculator* instance = static_cast<BLDCPulseCalculator*>(pvParameters);
  while (1) {
    instance->motorSpeed();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

