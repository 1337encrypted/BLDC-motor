#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class PwmGenerator{
  private:
  gpio_num_t pwmPin;
  uint32_t frequency;           // PWM frequency in Hz
  uint8_t resolution, dutyCycle;

  public:
  inline PwmGenerator(gpio_num_t=GPIO_NUM_NC, uint32_t=100000, uint8_t=8);
  inline void begin(const BaseType_t = 1);
  inline void front();
  inline uint8_t getPwm();
  inline void setPwm(uint8_t);

  static inline void frontTask(void*);
};

PwmGenerator::PwmGenerator(gpio_num_t pwmPin, uint32_t frequency, uint8_t resolution) :
pwmPin(pwmPin),
frequency(frequency),
resolution(resolution),
dutyCycle(0)
{}

void PwmGenerator::begin(const BaseType_t app_cpu) {

  //Task handles
  static TaskHandle_t frontHandle = nullptr;

  char *TAG = "PwmGenerator::begin";
  
  ledcAttach(this->pwmPin, frequency, resolution);

  if(!frontHandle) {
    BaseType_t result = xTaskCreatePinnedToCore(
      &frontTask,
      "frontTask",
      2048,
      this,
      1,
      &frontHandle,
      app_cpu
    );

    if (result == pdPASS) {
      ESP_LOGI("TAG", "Created the frontTask successfully");
    } else {
      ESP_LOGI("TAG", "Failed to create frontTask");
    }
  } else {
    ESP_LOGI("TAG", "frontTask already exists");
  }
}

void PwmGenerator::front()
{
  ledcWrite(this->pwmPin, this->dutyCycle);
} 

void PwmGenerator::frontTask(void* pvParameters){
  PwmGenerator* instance = static_cast<PwmGenerator*>(pvParameters);
  while (1) {
    instance->front();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

uint8_t PwmGenerator::getPwm(){
  return this->dutyCycle;
}
  
  
void PwmGenerator::setPwm(uint8_t dutyCycle){
  this->dutyCycle = dutyCycle;
}
