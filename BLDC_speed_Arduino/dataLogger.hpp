#pragma once
#include <BluetoothSerial.h>

class dataLogger {

    private:
    BluetoothSerial serialBT;
    char data[40];
    public:

    dataLogger(BluetoothSerial &);
    void begin(TaskHandle_t &, const BaseType_t = 0);
    void sendMotorData();

    //freeRTOS tasks
    static void dataLoggerTask(void *);
  };

  dataLogger::dataLogger(BluetoothSerial &serial) :
  serialBT(serial) // Initialize with the reference to the passed BluetoothSerial instance
  {
    memset(data, 0, sizeof(data));
  }


  void dataLogger::begin(TaskHandle_t &taskHandle, BaseType_t app_cpu) {
  
    const char* TAG = "dataLogger::begin";

    if(taskHandle == nullptr) {
        BaseType_t result = xTaskCreatePinnedToCore(
            &dataLoggerTask,
            "Data_Logger_Task",
            STACK_SIZE,
            this,
            1,
            &taskHandle,
            app_cpu
        );

        if (result == pdPASS){
            ESP_LOGI(TAG, "Created the dataLoggerTask successfully");
        } else {
            ESP_LOGI(TAG, "Failed to create the dataLoggerTask task");
        }
    } else {
        ESP_LOGI(TAG, "dataLoggerTask already created");
    }
  }

  void dataLogger::sendMotorData() {
    snprintf(data, sizeof(data), ",%s,%s,%s,%s,%s", OLEDFunctions::oledSpeed1, OLEDFunctions::oledSpeed2, OLEDFunctions::currentArr1, OLEDFunctions::currentArr2, OLEDFunctions::voltageArr1);
    serialBT.println(data);
  }

  void dataLogger::dataLoggerTask(void *pvParameters) {
    static dataLogger *instance = static_cast<dataLogger*>(pvParameters);
    while(true) {
      instance->sendMotorData();
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }