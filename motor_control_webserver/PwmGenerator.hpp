#pragma once

#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

class PwmGenerator {
  private:
  gpio_num_t pwmPin;
  uint32_t frequency;
  ledc_timer_bit_t resolution;
  ledc_timer_t timer;
  ledc_channel_t channel;
  uint8_t dutyCycle;
  static uint8_t timerCounter;
  static uint8_t channelCounter;

  public:
  PwmGenerator(gpio_num_t=GPIO_NUM_NC, uint32_t=100000, ledc_timer_bit_t=LEDC_TIMER_8_BIT);
  void begin(TaskHandle_t &, const BaseType_t app_cpu = 1);
  inline void updatePWM();
  
  inline uint8_t getPwm();
  inline void setPwm(uint8_t pwmValue);

  // freeRTOS task
  static void pwmTask(void* pvParameters);
};

uint8_t PwmGenerator::timerCounter = 0;
uint8_t PwmGenerator::channelCounter = 0;

PwmGenerator::PwmGenerator(gpio_num_t pwmPin, uint32_t frequency, ledc_timer_bit_t resolution) :
pwmPin(pwmPin),
frequency(frequency),
resolution(resolution),
dutyCycle(0)
{
  // Auto-assign timer and channel to avoid conflicts
  timer = static_cast<ledc_timer_t>(timerCounter++);
  channel = static_cast<ledc_channel_t>(channelCounter++);

  if (pwmPin == GPIO_NUM_NC) return; // Skip initialization if pin not configured

  ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_HIGH_SPEED_MODE,
    .duty_resolution = resolution,
    .timer_num = timer,
    .freq_hz = frequency,
    .clk_cfg = LEDC_AUTO_CLK
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
        .output_invert = 0
    }
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void PwmGenerator::begin(TaskHandle_t &taskHandle, const BaseType_t app_cpu) {
  if (pwmPin == GPIO_NUM_NC) return; // Skip if pin not configured

  if (taskHandle == nullptr) {
    BaseType_t result = xTaskCreatePinnedToCore(
      &pwmTask,
      "pwmTask",
      configMINIMAL_STACK_SIZE,
      this,
      1,
      &taskHandle,
      app_cpu
    );

    if (result != pdPASS) {
      ESP_LOGE("PwmGenerator::begin", "Failed to create PWM task");
    }
  }
}

void PwmGenerator::updatePWM() {
  if (pwmPin == GPIO_NUM_NC) return;
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
}

uint8_t PwmGenerator::getPwm() {
    return dutyCycle;
}

void PwmGenerator::setPwm(uint8_t pwmValue) {
  if (pwmPin == GPIO_NUM_NC) return;
  dutyCycle = pwmValue;
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, channel, dutyCycle);
  updatePWM();
}

void PwmGenerator::pwmTask(void* pvParameters) {
  PwmGenerator* instance = static_cast<PwmGenerator*>(pvParameters);
  while (true) {
    instance->updatePWM();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}