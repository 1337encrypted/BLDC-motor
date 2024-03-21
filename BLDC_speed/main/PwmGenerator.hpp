#pragma once

class PwmGenerator{
  private:
  gpio_num_t pwmPin;
  uint32_t frequency;           // PWM frequency in Hz
  uint8_t resolution, dutyCycle;

  public:
  inline PwmGenerator(gpio_num_t=GPIO_NUM_NC, uint32_t=100000, uint8_t=8);
  inline void begin(const BaseType_t = 1);
  inline void front();
  static inline void frontTask(void*);

  inline uint8_t getPwm();
  inline void setPwm(uint8_t);
};
