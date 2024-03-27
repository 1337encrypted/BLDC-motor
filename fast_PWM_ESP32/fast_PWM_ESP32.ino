#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class PwmGenerator{
  private:
  gpio_num_t motorPwmPin;
  uint32_t frequency;           // PWM frequency in Hz
  uint8_t resolution, dutyCycle;

  public:
  inline PwmGenerator(gpio_num_t=GPIO_NUM_NC, uint32_t=100000, uint8_t=8);
  inline void begin(TaskHandle_t &, const BaseType_t = 1);
  inline void front();

  inline uint8_t getPwm();
  inline void setPwm(uint8_t);

  // freeRTOS task
  static inline void frontTask(void*);
};

PwmGenerator::PwmGenerator(gpio_num_t motorPwmPin, uint32_t frequency, uint8_t resolution) :
motorPwmPin(motorPwmPin),
frequency(frequency),
resolution(resolution),
dutyCycle(0)
{}

void PwmGenerator::begin(TaskHandle_t &taskHandle, const BaseType_t app_cpu) {

  ledcAttach(this->motorPwmPin, frequency, resolution);

  char *TAG = "PwmGenerator::begin";

  if(taskHandle == nullptr) {
    BaseType_t result = xTaskCreatePinnedToCore(
      &frontTask,
      "frontTask",
      2048,
      this,
      1,
      &taskHandle,
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
  ledcWrite(this->motorPwmPin, this->dutyCycle);
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

PwmGenerator pwmChannel(GPIO_NUM_12, 100000, 8);
TaskHandle_t frontTaskHandle = nullptr;

void setup() {
  
  pwmChannel.begin(frontTaskHandle,1);
  pwmChannel.setPwm(255);

  vTaskDelete(NULL);
}

void loop() {

}
