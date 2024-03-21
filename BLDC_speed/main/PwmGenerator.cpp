#include "PwmGenerator.hpp"

PwmGenerator::PwmGenerator(gpio_num_t pwmPin, uint32_t frequency, uint8_t resolution) :
pwmPin(pwmPin),
frequency(frequency),
resolution(resolution),
dutyCycle(dutyCycle)
{
  this->dutyCycle = 0;
}

void PwmGenerator::begin(const BaseType_t app_cpu){
  ledcAttach(this->pwmPin, frequency, resolution);

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

void PwmGenerator::front()
{
  for(int i=25;i< 255;i=i+25)
  {
    ledcWrite(this->pwmPin, i);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
  ledcWrite(this->pwmPin, 0);
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