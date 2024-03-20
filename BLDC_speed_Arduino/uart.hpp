#pragma once

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

constexpr uint8_t TXD = 4;
constexpr uint8_t RXD = 5;
constexpr uint8_t RTS = (UART_PIN_NO_CHANGE);
constexpr uint8_t CTS = (UART_PIN_NO_CHANGE);

constexpr int UART_BAUD_RATE = 115200;

constexpr uart_port_t PORT_NUM = UART_NUM_2;
constexpr int STACK_SIZE = 2048;


class HardwareUart {
private:
    const int uart_buffer_size;
    QueueHandle_t uart_queue;
    int length;
    const char* test_str;

    // Define the structure for the motor characteristics data
    typedef struct MotorCharacteristicsData{
        float current1;
        float current2;
        float voltage1;
    }MotorCharacteristicsData;

    MotorCharacteristicsData received_data;
    
public:
    HardwareUart(int = 1024, const char* = "S\n");
    void begin(const BaseType_t = 1);
    void sendData();
    void receiveData();
    void printIncomingData(size_t);

    //FreeRTOS task
    static void uartTask(void *);

    TaskHandle_t xUartHandle;
};

HardwareUart::HardwareUart(int buffer_size, const char* str) : 
uart_buffer_size(buffer_size), 
length(0), 
test_str(str),
received_data{0.00, 0.00, 0.00} {

    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = UART_SCLK_APB,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(uart_param_config(PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(PORT_NUM, TXD, RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(PORT_NUM, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));

    xUartHandle = nullptr;
}

void HardwareUart::begin(const BaseType_t app_cpu) {
    if(!xUartHandle) {
        BaseType_t result = xTaskCreatePinnedToCore(
            uartTask,
            "uart_task",
            STACK_SIZE,
            this,
            2,
            &xUartHandle,
            app_cpu
        );

        if (result == pdPASS){
            ESP_LOGI("UARTBEGIN", "Created the UartTask successfully");
        } else {
            ESP_LOGI("UARTBEGIN", "Failed to create the UartTask task");
        }
    } else {
        ESP_LOGI("UARTBEGIN", "UartTask already created");
    }
}

void HardwareUart::sendData() {
    uart_write_bytes(PORT_NUM, test_str, strlen(test_str));
}


void HardwareUart::receiveData() {

    static const char* TAG = "receiveData"; // Define TAG here
    
    // Define a buffer to store received data
    uint8_t data_buffer[sizeof(MotorCharacteristicsData)];

    // Read the data from the UART buffer
    size_t data_length = uart_read_bytes(PORT_NUM, data_buffer, sizeof(data_buffer), 1500 / portTICK_PERIOD_MS);

    if (data_length == sizeof(MotorCharacteristicsData)) {  
        memcpy(&received_data, data_buffer, sizeof(MotorCharacteristicsData)); // Deserialize the received bytes into a MotorCharacteristicsData struct
        // printIncomingData(data_length);
    } else {
        ESP_LOGW(TAG, "Received data size does not match struct size");
    }
}

void HardwareUart::printIncomingData(size_t data_length) {

        static const char* TAG = "printIncomingData"; // Define TAG here

        ESP_LOGI(TAG, "Incoming data Length: %d", data_length);
        ESP_LOGI(TAG, "Data Structure Length: %d", sizeof(MotorCharacteristicsData));

        //Now you can access the individual members of the received_data struct
        ESP_LOGI(TAG, "Received current1: %f", received_data.current1);
        ESP_LOGI(TAG, "Received current2: %f", received_data.current2);
        ESP_LOGI(TAG, "Received voltage1: %f", received_data.voltage1);
} 



void HardwareUart::uartTask(void *arg) {
    HardwareUart *serial = static_cast<HardwareUart*>(arg);

    while (1) {
        serial->sendData();
        serial->receiveData();
    }
}