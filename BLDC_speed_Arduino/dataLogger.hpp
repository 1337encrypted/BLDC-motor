#include <BluetoothSerial.h>

class dataLogger {

    private:
    TaskHandle_t dataLoggerTaskHandler;
    BluetoothSerial SerialBT;
    char data[40];
    public:

    dataLogger(BluetoothSerial &);
    void begin(BaseType_t);
    void sendMotorData();

    //freeRTOS tasks
    static void dataLoggerTask(void *);
  };

  dataLogger::dataLogger(BluetoothSerial &serial) :
  dataLoggerTaskHandler(nullptr),
  SerialBT(serial) // Initialize with the reference to the passed BluetoothSerial instance
  {
    memset(data, 0, sizeof(data));
  }


  void dataLogger::begin(BaseType_t app_cpu) {

    // dataLoggerTaskHandler = nullptr;
  
    const char* TAG = "dataLogger::begin";

    if(dataLoggerTaskHandler == nullptr) {
        BaseType_t result = xTaskCreatePinnedToCore(
            &dataLoggerTask,
            "Data_Logger_Task",
            STACK_SIZE,
            this,
            1,
            &dataLoggerTaskHandler,
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
    SerialBT.println(data);
  }

  void dataLogger::dataLoggerTask(void *pvParameters) {
    static dataLogger *instance = static_cast<dataLogger*>(pvParameters);
    while(true) {
      instance->sendMotorData();
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
  }