#pragma once

#include <driver/gpio.h>
#include <esp_log.h>
#include <cstring>
#include "esp_timer.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

class BLDCPulseCalculator {
private:
  portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;

  gpio_num_t wavePin;
  uint8_t motorId;

  volatile uint32_t timePeriodValues[32];
  volatile uint32_t newTimePeriodValues[16];

  volatile uint16_t speed;

  enum class BLDCstates : uint8_t {
    IDLE,
    PRINT
  };
  volatile BLDCstates status = BLDCstates::IDLE;

  static BLDCPulseCalculator* instance;

public:

  BLDCPulseCalculator(gpio_num_t wavePin = GPIO_NUM_NC, uint8_t motorId = -1);
  void calculateValuesInternal(void);
  void motorSpeed();
  
  void begin(const BaseType_t = 1);
  static void motorSpeedTask(void*);
  static void staticCalculateValuesWrapper(void *);
};

BLDCPulseCalculator* BLDCPulseCalculator::instance = nullptr;

BLDCPulseCalculator::BLDCPulseCalculator(gpio_num_t wavePin, uint8_t motorId) :
  wavePin(wavePin), 
  motorId(motorId),
  speed(0)
{
  memset((void *)(timePeriodValues), 0, sizeof(timePeriodValues));
  memset((void *)(newTimePeriodValues), 0, sizeof(newTimePeriodValues));

  instance = this;
}

void BLDCPulseCalculator::begin(const BaseType_t app_cpu) {

  gpio_config_t gpioOutputConfigure = {
    .pin_bit_mask = (1ULL << wavePin),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_ANYEDGE
  };
  ESP_ERROR_CHECK(gpio_config(&gpioOutputConfigure));

  // gpio_set_direction(wavePin, GPIO_MODE_INPUT);
  // gpio_pulldown_en(wavePin);
  // gpio_pullup_dis(wavePin);
  // gpio_set_intr_type(wavePin, GPIO_INTR_ANYEDGE);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(this->wavePin, staticCalculateValuesWrapper, (void*)(this->instance));

  xTaskCreatePinnedToCore(
    motorSpeedTask,
    "motorSpeedTask",
    2048,
    this->instance,
    1,
    NULL,
    app_cpu
  );
}

void BLDCPulseCalculator::calculateValuesInternal() {
  static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  portENTER_CRITICAL_ISR(&mux);

  static volatile uint8_t counter = 0;
  static volatile unsigned long nextTimeStamp;
  volatile unsigned long timeStamp;

  timeStamp = esp_timer_get_time() / 1000;  // Convert to milliseconds
  timePeriodValues[counter] = static_cast<uint32_t>(timeStamp - nextTimeStamp);
  counter = counter + 1;
  nextTimeStamp = timeStamp;

  if ((counter % 2) == 0 && counter != 0) {
    newTimePeriodValues[counter / 2 - 1] = timePeriodValues[counter - 1] + timePeriodValues[counter - 2];
  }

  if (counter == 32) {
    counter = 0;
    status = BLDCstates::PRINT;
  }

  portEXIT_CRITICAL_ISR(&mux);
}

void BLDCPulseCalculator::motorSpeed() {
  uint32_t sumTime = 0;

  switch (status) {
    case BLDCstates::PRINT:
      for (int i = 0; i < 16; i++) {
        sumTime += newTimePeriodValues[i];
      }

      // timePeriod = (uint32_t *)(sumTime / 16.0);
      // speed = (uint16_t)(3750 / timePeriod);

      speed = static_cast<uint16_t>(60000 / sumTime);
      Serial.println(speed);
      // ESP_LOGI("MOTOR", "Speed %u", speed);
      sumTime = 0;
      status = BLDCstates::IDLE;
      break;

    case BLDCstates::IDLE:
      break;
  }
}

void IRAM_ATTR BLDCPulseCalculator::staticCalculateValuesWrapper(void *args) {
  BLDCPulseCalculator* instance = static_cast<BLDCPulseCalculator*>(args);
  if (instance) {
    instance->calculateValuesInternal();
  }
}

void BLDCPulseCalculator::motorSpeedTask(void* pvParameters) {
  BLDCPulseCalculator* instance = static_cast<BLDCPulseCalculator*>(pvParameters);
  while (1) {
    instance->motorSpeed();
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

