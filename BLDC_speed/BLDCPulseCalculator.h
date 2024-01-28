#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class BLDCPulseCalculator {
private:
  portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;
  volatile unsigned long timeStamp;
  volatile unsigned long nextTimeStamp;
  volatile unsigned long timePeriodValues[32];
  volatile unsigned long newTimePeriodValues[16];
  volatile uint8_t counter;
  volatile unsigned long sumTime;
  volatile unsigned long timePeriod;
  volatile unsigned long speed;

  int8_t wavePin;
  uint8_t motorId;

  enum class BLDCstates : uint8_t {
    IDLE,
    PRINT
  };
  volatile BLDCstates status = BLDCstates::IDLE;

  
  static BLDCPulseCalculator* instance;                                                               // Static wrapper function for interrupt

  static void IRAM_ATTR staticCalculateValuesWrapper();
  inline void IRAM_ATTR calculateValuesInternal() __attribute__((always_inline));                     // Member function for interrupt handling
  inline static void motorSpeedTask(void* pvParameters) __attribute__((always_inline));               // FreeRTOS task function for motor speed calculation
  inline void motorSpeed() __attribute__((always_inline));

public:
  inline BLDCPulseCalculator(int8_t wavePin = -1, uint8_t = -1)  __attribute__((always_inline));
  inline void begin(const BaseType_t = 1)  __attribute__((always_inline));
};

// Initialize the static pointer to null
BLDCPulseCalculator* BLDCPulseCalculator::instance = nullptr;

BLDCPulseCalculator::BLDCPulseCalculator(int8_t wavePin, uint8_t motorId) : 
wavePin(wavePin),
motorId(motorId)
{
  timeStamp = 0;
  nextTimeStamp = 0;
  timePeriod = 0;
  sumTime = 0;
  speed = 0;
  counter = 0;

  // Use non-volatile pointers for memset
  memset((void*)timePeriodValues, 0, sizeof(timePeriodValues));
  memset((void*)newTimePeriodValues, 0, sizeof(newTimePeriodValues));

  // Assign the instance pointer to the current instance
  instance = this;
}

void BLDCPulseCalculator::begin(const BaseType_t app_cpu) {
  pinMode(this->wavePin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(this->wavePin), staticCalculateValuesWrapper, CHANGE);

  // Create a FreeRTOS task for motorSpeed() with priority 1  
  xTaskCreatePinnedToCore(        // Use xTaskCreate() in vanilla FreeRTOS
            motorSpeedTask,       // Function to be called
            "motorSpeedTask",     // Name of task
            2048,                 // Stack size (bytes in ESP32, words in FreeRTOS)
            this,                 // Parameter to pass
            1,                    // Task priority
            NULL,                 // Task handle
            app_cpu);             // Run on one core for demo purposes (ESP32 only)
}

// Static wrapper function
void IRAM_ATTR BLDCPulseCalculator::staticCalculateValuesWrapper() {
  // Check if the instance pointer is not null
  if (instance) {
    // Call the non-static member function through the instance pointer
    instance->calculateValuesInternal();
  }
}

void IRAM_ATTR BLDCPulseCalculator::calculateValuesInternal() {
  portENTER_CRITICAL_ISR(&synch);

  timeStamp = millis();
  timePeriodValues[counter++] = timeStamp - nextTimeStamp;
  nextTimeStamp = timeStamp;

  if ((counter % 2) == 0 && counter != 0) {
    newTimePeriodValues[counter / 2 - 1] = timePeriodValues[counter - 1] + timePeriodValues[counter - 2];
  }

  if (counter == 32) {
    counter = 0;
    status = BLDCstates::PRINT;
  }

  portEXIT_CRITICAL_ISR(&synch);
}

void BLDCPulseCalculator::motorSpeed() {
  switch (status) {
    case BLDCstates::PRINT:
      for (int i = 0; i < 16; i++) {
        sumTime += newTimePeriodValues[i];
      }
      timePeriod = (unsigned long)(sumTime / 16.0);
      speed = (unsigned long)(3750 / timePeriod);

      // Serial.print(speed);
      // Serial.println();

      ESP_LOGI(motorId, "\tSpeed %d", speed);


      sumTime = 0;

      status = BLDCstates::IDLE;
      break;

    case BLDCstates::IDLE:
      break;
  }
}

// FreeRTOS task function for motor speed calculation
void BLDCPulseCalculator::motorSpeedTask(void* pvParameters) {
  BLDCPulseCalculator* bldcInstance = static_cast<BLDCPulseCalculator*>(pvParameters);
  while(1)
  {
    bldcInstance->motorSpeed();
    vTaskDelay(pdMS_TO_TICKS(100)); // Adjust the delay as needed
  }
}