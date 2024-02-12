#pragma once
// #include <driver/gpio.h>
#include "driver/ledc.h"
#include <cstring>
#include <esp_log.h>
#include "esp_err.h"
#include "esp_timer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "oledFunctions.hpp"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (100000) // Frequency in Hertz. Set frequency at 100 kHz

class motorSynchronization{
  private:
  gpio_num_t motorPwmPin;
  // uint8_t speed;

  public:
  inline motorSynchronization(gpio_num_t = GPIO_NUM_NC) __attribute__((always_inline));
  inline void begin(const BaseType_t = 1) __attribute__((always_inline));
  inline void front() __attribute__((always_inline));

  //RTOS tasks
  static inline void frontTask(void*) __attribute__((always_inline));
};

motorSynchronization::motorSynchronization(gpio_num_t motorPwmPin) :
motorPwmPin(motorPwmPin)
{}

void motorSynchronization::begin(const BaseType_t app_cpu){

  // Prepare and then apply the LEDC PWM timer configuration
  ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_MODE,
    .duty_resolution  = LEDC_DUTY_RES,
    .timer_num        = LEDC_TIMER,
    .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
    .clk_cfg          = LEDC_AUTO_CLK
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel = {
    .gpio_num       = this->motorPwmPin,
    .speed_mode     = LEDC_MODE,
    .channel        = LEDC_CHANNEL,
    .intr_type      = LEDC_INTR_DISABLE,
    .timer_sel      = LEDC_TIMER,
    .duty           = 0, // Set duty to 0%
    .hpoint         = 0
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

  xTaskCreatePinnedToCore(
    frontTask,
    "frontTask",
    2048,
    this,
    1,
    NULL,
    app_cpu
  );
}

void motorSynchronization::front(){
  // Set duty to 50%
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
    // Update duty to apply the new value
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}

void motorSynchronization::frontTask(void* pvParameters){
  motorSynchronization* instance = static_cast<motorSynchronization*>(pvParameters);
  while (1) {
    instance->front();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
