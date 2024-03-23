#pragma once

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
// #include "oledFunctions.hpp"

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

    inline void sendData() __attribute__((always_inline));
    inline void receiveData() __attribute__((always_inline));
    inline void printIncomingData(size_t) __attribute__((always_inline));

    //FreeRTOS task
    static inline void uartTask(void *) __attribute__((always_inline));

    TaskHandle_t xUartHandle;
};