#pragma once

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

// UART Configuration for Current Sensor Communication  
// Using pins from GLOBALS.hpp to avoid conflicts
constexpr uint8_t CURRENT_SENSOR_TXD = 17;  // UART_TX_PIN from GLOBALS.hpp
constexpr uint8_t CURRENT_SENSOR_RXD = 16;  // UART_RX_PIN from GLOBALS.hpp
constexpr uint8_t CURRENT_SENSOR_RTS = (UART_PIN_NO_CHANGE);
constexpr uint8_t CURRENT_SENSOR_CTS = (UART_PIN_NO_CHANGE);

constexpr int CURRENT_SENSOR_BAUD_RATE = 115200;
constexpr uart_port_t CURRENT_SENSOR_PORT = UART_NUM_2;
constexpr int CURRENT_SENSOR_STACK_SIZE = 2048;

class UARTCurrentSensor {
private:
    const int uart_buffer_size;
    QueueHandle_t uart_queue;
    const char* request_command;
    
    // Current sensor data structure (matches Arduino Nano)
    typedef struct MotorCharacteristicsData {
        float current1;    // Motor 1 current in Amps
        float current2;    // Motor 2 current in Amps
        float voltage1;    // System voltage in Volts
    } MotorCharacteristicsData;

    MotorCharacteristicsData sensor_data;
    bool data_valid;
    unsigned long last_update_time;
    
public:
    UARTCurrentSensor(int buffer_size = 1024, const char* cmd = "S");
    void begin(TaskHandle_t &taskHandle, const BaseType_t app_cpu = 1);
    void requestData();
    void receiveData();
    
    // Getter functions for sensor data
    float getCurrent1() const { return data_valid ? sensor_data.current1 : 0.0f; }
    float getCurrent2() const { return data_valid ? sensor_data.current2 : 0.0f; }
    float getVoltage() const { return data_valid ? sensor_data.voltage1 : 0.0f; }
    bool isDataValid() const { return data_valid; }
    unsigned long getLastUpdateTime() const { return last_update_time; }
    
    // Debug functions
    void printSensorData();
    void sendDebugCommand();
    void sendResetCommand();
    
    // FreeRTOS task
    static void uartCurrentSensorTask(void *pvParameters);
};

UARTCurrentSensor::UARTCurrentSensor(int buffer_size, const char* cmd) : 
uart_buffer_size(buffer_size),
request_command(cmd),
sensor_data{0.0f, 0.0f, 0.0f},
data_valid(false),
last_update_time(0) {

    // Configure UART
    uart_config_t uart_config = {
        .baud_rate = CURRENT_SENSOR_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(uart_param_config(CURRENT_SENSOR_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CURRENT_SENSOR_PORT, CURRENT_SENSOR_TXD, CURRENT_SENSOR_RXD, 
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(CURRENT_SENSOR_PORT, uart_buffer_size, uart_buffer_size, 
                                       10, &uart_queue, 0));
}

void UARTCurrentSensor::begin(TaskHandle_t &taskHandle, const BaseType_t app_cpu) {
    const char* TAG = "UARTCurrentSensor::begin";

    if(taskHandle == nullptr) {
        BaseType_t result = xTaskCreatePinnedToCore(
            &uartCurrentSensorTask,
            "uart_current_sensor_task",
            CURRENT_SENSOR_STACK_SIZE,
            this,
            2,
            &taskHandle,
            app_cpu
        );
        
        if (result == pdPASS) {
            ESP_LOGI(TAG, "Created the uart_current_sensor_task successfully");
        } else {
            ESP_LOGE(TAG, "Failed to create the uart_current_sensor_task");
        }
    } else {
        ESP_LOGI(TAG, "uart_current_sensor_task already created");
    }
}

void UARTCurrentSensor::requestData() {
    uart_write_bytes(CURRENT_SENSOR_PORT, request_command, strlen(request_command));
}

void UARTCurrentSensor::receiveData() {
    const char* TAG = "UARTCurrentSensor::receiveData";
    
    // Buffer to store received data
    uint8_t data_buffer[sizeof(MotorCharacteristicsData)];
    
    // Read data from UART buffer with timeout
    size_t data_length = uart_read_bytes(CURRENT_SENSOR_PORT, data_buffer, 
                                        sizeof(data_buffer), 200 / portTICK_PERIOD_MS);
    
    if (data_length == sizeof(MotorCharacteristicsData)) {
        // Deserialize received bytes into struct
        memcpy(&sensor_data, data_buffer, sizeof(MotorCharacteristicsData));
        data_valid = true;
        last_update_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        ESP_LOGD(TAG, "Received valid sensor data: I1=%.2fA, I2=%.2fA, V=%.1fV", 
                sensor_data.current1, sensor_data.current2, sensor_data.voltage1);
    } else if (data_length > 0) {
        ESP_LOGW(TAG, "Received incorrect data size: %d bytes (expected %d)", 
                data_length, sizeof(MotorCharacteristicsData));
        // Flush remaining data
        uart_flush(CURRENT_SENSOR_PORT);
    }
    
    // Mark data as invalid if no updates for more than 5 seconds
    if (data_valid && (xTaskGetTickCount() * portTICK_PERIOD_MS - last_update_time) > 5000) {
        data_valid = false;
        ESP_LOGW(TAG, "Current sensor data timeout - marking as invalid");
    }
}

void UARTCurrentSensor::printSensorData() {
    if (data_valid) {
        ESP_LOGI("CurrentSensor", "Motor1: %.2fA, Motor2: %.2fA, Voltage: %.1fV", 
                sensor_data.current1, sensor_data.current2, sensor_data.voltage1);
    } else {
        ESP_LOGW("CurrentSensor", "No valid sensor data available");
    }
}

void UARTCurrentSensor::sendDebugCommand() {
    uart_write_bytes(CURRENT_SENSOR_PORT, "D", 1);
}

void UARTCurrentSensor::sendResetCommand() {
    uart_write_bytes(CURRENT_SENSOR_PORT, "R", 1);
}

void UARTCurrentSensor::uartCurrentSensorTask(void *pvParameters) {
    UARTCurrentSensor *sensor = static_cast<UARTCurrentSensor*>(pvParameters);
    
    while (1) {
        sensor->requestData();
        vTaskDelay(50 / portTICK_PERIOD_MS);  // Wait for Arduino to process
        sensor->receiveData();
        vTaskDelay(100 / portTICK_PERIOD_MS); // 10Hz update rate
    }
}