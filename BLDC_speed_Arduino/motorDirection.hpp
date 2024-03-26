#pragma once
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

class MotorDirection {

    private:
    gpio_num_t triggerPin1, triggerPin2;

    public:
    MotorDirection(gpio_num_t=GPIO_NUM_NC, gpio_num_t=GPIO_NUM_NC) :
    triggerPin1(triggerPin1),
    triggerPin2(triggerPin2) 
    {}

  void begin() {
    gpio_config_t gpioOutputConfigure = {
        .pin_bit_mask = (1ULL << this->triggerPin1) | (1ULL << this->triggerPin2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
      };
      ESP_ERROR_CHECK(gpio_config(&gpioOutputConfigure));
  }

  void selfLearning() {
    gpio_set_level(triggerPin1, 1);
    gpio_set_level(triggerPin2, 1);
  }

  void stopSelfLearning() {
    gpio_set_level(triggerPin1, 0);
    gpio_set_level(triggerPin2, 0);
  }
};