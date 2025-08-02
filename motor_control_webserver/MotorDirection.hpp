#pragma once
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

// Future implementation for motor direction control
class MotorDirection {
    private:
    gpio_num_t triggerPin1, triggerPin2;
    bool directionState;

    public:
    MotorDirection(gpio_num_t=GPIO_NUM_NC, gpio_num_t=GPIO_NUM_NC);
    void begin();
    void setDirection(bool forward);
    void toggleDirection();
    bool getDirection();
};

MotorDirection::MotorDirection(gpio_num_t triggerPin1, gpio_num_t triggerPin2) :
triggerPin1(triggerPin1),
triggerPin2(triggerPin2),
directionState(true) // true = forward, false = reverse
{}

void MotorDirection::begin() {
  if (triggerPin1 == GPIO_NUM_NC || triggerPin2 == GPIO_NUM_NC) return;
  
  gpio_config_t gpioOutputConfigure = {
      .pin_bit_mask = (1ULL << this->triggerPin1) | (1ULL << this->triggerPin2),
      .mode = GPIO_MODE_OUTPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE
  };
  ESP_ERROR_CHECK(gpio_config(&gpioOutputConfigure));
  
  // Initialize in forward direction
  setDirection(true);
}

void MotorDirection::setDirection(bool forward) {
  if (triggerPin1 == GPIO_NUM_NC || triggerPin2 == GPIO_NUM_NC) return;
  
  directionState = forward;
  gpio_set_level(triggerPin1, forward ? 0 : 1);
  gpio_set_level(triggerPin2, forward ? 0 : 1);
  ESP_LOGI("MotorDirection", "Direction set to %s", forward ? "forward" : "reverse");
}

void MotorDirection::toggleDirection() {
  setDirection(!directionState);
}

bool MotorDirection::getDirection() {
  return directionState;
}