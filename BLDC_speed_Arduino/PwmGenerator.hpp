#pragma once

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include "esp_log.h"

class PwmGenerator {
  private:
  gpio_num_t pwmPin;
  uint32_t frequency;
  ledc_timer_bit_t resolution;
  ledc_timer_t timer;
  ledc_channel_t channel;
  uint8_t dutyCycle;

  public:
  PwmGenerator(gpio_num_t=GPIO_NUM_NC, uint32_t=100000, ledc_timer_bit_t=LEDC_TIMER_8_BIT);
  void begin(TaskHandle_t &, const BaseType_t app_cpu = 1);
  inline void front();
  
  inline uint8_t getPwm();
  inline void setPwm(uint8_t PwmGeneratorValue);

  // freeRTOS task
  static void frontTask(void* pvParameters);
};


PwmGenerator::PwmGenerator(gpio_num_t pwmPin, uint32_t frequency, ledc_timer_bit_t resolution) :
pwmPin(pwmPin),
frequency(frequency),
resolution(resolution),
dutyCycle(0)
{
  // Initialize timer and channel
  timer = LEDC_TIMER_0; // Choose the timer number
  channel = LEDC_CHANNEL_0; // Choose the channel number

  ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = resolution,
    .timer_num = timer,
    .freq_hz = frequency,
    .clk_cfg = LEDC_AUTO_CLK
    // .deconfigure = false // Do not de-configure the timer
  };
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  ledc_channel_config_t ledc_channel = {
    .gpio_num = pwmPin,
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .channel = channel,
    .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = timer,
    .duty = 0,
    .hpoint = 0,
    .flags = {
        .output_invert = 0 // Disable GPIO output invert
    }
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void PwmGenerator::begin(TaskHandle_t &taskHandle, const BaseType_t app_cpu) {
  // Create a task to handle PwmGenerator updates
  if (taskHandle == nullptr) {
    BaseType_t result = xTaskCreatePinnedToCore(
      &frontTask,
      "frontTask",
      configMINIMAL_STACK_SIZE,
      this,
      1,
      &taskHandle,
      app_cpu
    );

    if (result != pdPASS) {
      ESP_LOGE("PwmGenerator::begin", "Failed to create PwmGenerator task");
    }
  }
}

void PwmGenerator::front() {
  // Update PwmGenerator duty cycle
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
  vTaskDelay(pdMS_TO_TICKS(10));
}

uint8_t PwmGenerator::getPwm() {
    return dutyCycle;
}

void PwmGenerator::setPwm(uint8_t dutyCycle) {
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, dutyCycle);
}

void PwmGenerator::frontTask(void* pvParameters) {
  PwmGenerator* instance = static_cast<PwmGenerator*>(pvParameters);
  while (true) {
    instance->front();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
