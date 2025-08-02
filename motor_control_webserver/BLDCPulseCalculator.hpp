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
  gpio_num_t feedBackPin;
  uint8_t motorId;

  volatile uint8_t counter;
  volatile unsigned long nextTimeStamp;
  volatile unsigned long timeStamp;

  volatile uint32_t timePeriodValues[32];
  volatile uint32_t newTimePeriodValues[16];

  volatile uint16_t speed;

  enum class BLDCstates : uint8_t {
    IDLE,
    PRINT
  };
  volatile BLDCstates status = BLDCstates::IDLE;

public:
  inline BLDCPulseCalculator(gpio_num_t feedBackPin = GPIO_NUM_NC, uint8_t motorId = -1) __attribute__((always_inline));
  inline void calculateValuesInternal(void) __attribute__((always_inline));
  inline void motorSpeed() __attribute__((always_inline));

  inline uint16_t getSpeed() __attribute__((always_inline));
  inline void begin(TaskHandle_t &, const BaseType_t = 1) __attribute__((always_inline));

  // FreeRTOS
  static inline void motorSpeedTask(void*) __attribute__((always_inline));
  static void staticCalculateValuesWrapper(void *);
};

BLDCPulseCalculator::BLDCPulseCalculator(gpio_num_t feedBackPin, uint8_t motorId) :
feedBackPin(feedBackPin), 
motorId(motorId),
speed(0),
counter(0),
nextTimeStamp(0),
timeStamp(0)
{
  memset((void *)(timePeriodValues), 0, sizeof(timePeriodValues));
  memset((void *)(newTimePeriodValues), 0, sizeof(newTimePeriodValues));
}

void BLDCPulseCalculator::begin(TaskHandle_t &taskHandle, const BaseType_t app_cpu) {
  if (feedBackPin == GPIO_NUM_NC) return; // Skip if pin not configured

  char *TAG = "BLDCPulseCalculator::begin";

  gpio_config_t gpioOutputConfigure = {
    .pin_bit_mask = (1ULL << this->feedBackPin),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_ENABLE,
    .intr_type = GPIO_INTR_ANYEDGE
  };
  ESP_ERROR_CHECK(gpio_config(&gpioOutputConfigure));

  gpio_install_isr_service(0);
  gpio_isr_handler_add(this->feedBackPin, staticCalculateValuesWrapper, (void*)(this));

  if(taskHandle == nullptr) {
    BaseType_t result = xTaskCreatePinnedToCore(
      &motorSpeedTask,
      "motorSpeedTask",
      2048,
      this,
      1,
      &taskHandle,
      app_cpu
    );

    if (result == pdPASS){
        ESP_LOGI(TAG, "Created the motorSpeedTask successfully for motor %d", motorId);
    } else {
        ESP_LOGE(TAG, "Failed to create the motorSpeedTask task for motor %d", motorId);
    }
  } else {
    ESP_LOGI(TAG, "motorSpeedTask already created for motor %d", motorId);
  }
}

void BLDCPulseCalculator::calculateValuesInternal() {
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
}

void BLDCPulseCalculator::motorSpeed() {
  uint32_t sumTime = 0;

  switch (status) {
    case BLDCstates::PRINT:
      for (int i = 0; i < 16; i++) {
        sumTime += newTimePeriodValues[i];
      }

      speed = static_cast<uint16_t>(60000 / sumTime);
      ESP_LOGD("MOTOR", "Motor %d Speed: %u RPM", motorId, speed);

      sumTime = 0;
      status = BLDCstates::IDLE;
      break;

    case BLDCstates::IDLE:
      // Set speed to 0 when wheel is not moving for 2 seconds
      if(((esp_timer_get_time() / 1000) - nextTimeStamp) > 2000)
      {
        speed = 0;
      }
      break;
  }
}

uint16_t BLDCPulseCalculator::getSpeed() {
  return speed;
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