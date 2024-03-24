#pragma once
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

class MotorDirection {

    private:
    gpio_num_t relayPin1, relayPin2;

    public:
    MotorDirection(gpio_num_t=GPIO_NUM_NC, gpio_num_t=GPIO_NUM_NC) :
    relayPin1(relayPin1),
    relayPin2(relayPin2) 
    {}

  void begin() {
    gpio_config_t gpioOutputConfigure = {
        .pin_bit_mask = (1ULL << this->relayPin1) | (1ULL << this->relayPin2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
      };
      ESP_ERROR_CHECK(gpio_config(&gpioOutputConfigure));
  }

  void front() {
    gpio_set_level(relayPin1, 0);
    gpio_set_level(relayPin2, 0);
  }

  void back() {
    gpio_set_level(relayPin1, 1);
    gpio_set_level(relayPin2, 1);
  }
};