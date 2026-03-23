#pragma once

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include <numeric>
#include <vector>

// UART Configuration for Current Sensor Communication
constexpr uint8_t CURRENT_SENSOR_TXD = 4;   // Updated to match GLOBALS.hpp
constexpr uint8_t CURRENT_SENSOR_RXD = 5;   // Updated to match GLOBALS.hpp
constexpr int CURRENT_SENSOR_BAUD_RATE = 115200;
constexpr uart_port_t CURRENT_SENSOR_PORT = UART_NUM_2;
constexpr int CURRENT_SENSOR_STACK_SIZE = 4096; // Increased stack size for more complex logic

// --- New Robust Communication Protocol ---
constexpr uint8_t SOF_BYTE = 0xAA; // Start of Frame
constexpr uint8_t EOF_BYTE = 0x55; // End of Frame

// Data structure for communication (must match Arduino Nano)
#pragma pack(push, 1)
typedef struct MotorCharacteristicsData {
    float current1;
    float current2;
    float voltage1;
} MotorCharacteristicsData;
#pragma pack(pop)

// Full packet structure including SOF, EOF, and checksum
#pragma pack(push, 1)
struct UARTPacket {
    uint8_t sof;
    MotorCharacteristicsData data;
    uint8_t checksum;
    uint8_t eof;
};
#pragma pack(pop)
// --- End of Protocol Definition ---

// --- Data Smoothing ---
template<typename T, size_t N>
class MovingAverage {
private:
    T buffer[N];
    size_t index = 0;
    T sum = 0;
    bool full = false;

public:
    MovingAverage() {
        for(size_t i = 0; i < N; ++i) buffer[i] = 0;
    }

    void add(T value) {
        sum -= buffer[index];
        buffer[index] = value;
        sum += value;
        index = (index + 1) % N;
        if (index == 0) full = true;
    }

    T get() const {
        return sum / (full ? N : (index == 0 ? 1 : index));
    }
};
// --- End of Data Smoothing ---

class UARTCurrentSensor {
private:
    const int uart_buffer_size;
    QueueHandle_t uart_queue;
    const char* request_command;

    MotorCharacteristicsData sensor_data;
    bool data_valid;
    unsigned long last_update_time;

    // Data smoothing filters
    MovingAverage<float, 5> current1_filter;
    MovingAverage<float, 5> current2_filter;
    MovingAverage<float, 5> voltage_filter;

    uint8_t calculateChecksum(const MotorCharacteristicsData& data) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&data);
        uint8_t checksum = 0;
        for (size_t i = 0; i < sizeof(data); ++i) {
            checksum ^= bytes[i];
        }
        return checksum;
    }

public:
    UARTCurrentSensor(int buffer_size = 2048, const char* cmd = "S");
    void begin(TaskHandle_t &taskHandle, const BaseType_t app_cpu = 1);
    void requestData();
    bool receiveData(); // Changed to return bool for success

    // Getters now return smoothed data
    float getCurrent1() const { return data_valid ? current1_filter.get() : 0.0f; }
    float getCurrent2() const { return data_valid ? current2_filter.get() : 0.0f; }
    float getVoltage() const { return data_valid ? voltage_filter.get() : 0.0f; }
    bool isDataValid() const { return data_valid; }
    unsigned long getLastUpdateTime() const { return last_update_time; }

    void sendIPAddress(const String& ipAddress);
    void sendWiFiMode(bool isAPMode, const String& ipAddress);
    void sendWiFiStrength(int rssi);

    static void uartCurrentSensorTask(void *pvParameters);
};

UARTCurrentSensor::UARTCurrentSensor(int buffer_size, const char* cmd) :
uart_buffer_size(buffer_size),
request_command(cmd),
sensor_data{0.0f, 0.0f, 0.0f},
data_valid(false),
last_update_time(0) {
    uart_config_t uart_config = {
        .baud_rate = CURRENT_SENSOR_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };
    ESP_ERROR_CHECK(uart_param_config(CURRENT_SENSOR_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(CURRENT_SENSOR_PORT, CURRENT_SENSOR_TXD, CURRENT_SENSOR_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(CURRENT_SENSOR_PORT, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0));
}

void UARTCurrentSensor::begin(TaskHandle_t &taskHandle, const BaseType_t app_cpu) {
    const char* TAG = "UARTCurrentSensor::begin";
    if(taskHandle == nullptr) {
        BaseType_t result = xTaskCreatePinnedToCore(&uartCurrentSensorTask, "uart_current_sensor_task", CURRENT_SENSOR_STACK_SIZE, this, 2, &taskHandle, app_cpu);
        if (result != pdPASS) {
            ESP_LOGE(TAG, "Failed to create the uart_current_sensor_task");
        }
    }
}

void UARTCurrentSensor::requestData() {
    uart_write_bytes(CURRENT_SENSOR_PORT, request_command, strlen(request_command));
}

bool UARTCurrentSensor::receiveData() {
    const char* TAG = "UARTCurrentSensor::receiveData";
    uint8_t buffer[sizeof(UARTPacket)];
    int bytes_read = 0;
    
    // 1. Find Start of Frame (SOF)
    uint8_t sof_finder;
    do {
        bytes_read = uart_read_bytes(CURRENT_SENSOR_PORT, &sof_finder, 1, pdMS_TO_TICKS(100));
        if (bytes_read == 0) {
            ESP_LOGD(TAG, "Timeout waiting for SOF byte - checking Arduino connection");
            return false;
        }
    } while (sof_finder != SOF_BYTE);

    // 2. Read the rest of the packet
    buffer[0] = SOF_BYTE;
    int remaining_bytes = sizeof(UARTPacket) - 1;
    bytes_read = uart_read_bytes(CURRENT_SENSOR_PORT, &buffer[1], remaining_bytes, pdMS_TO_TICKS(100));

    if (bytes_read != remaining_bytes) {
        ESP_LOGW(TAG, "Incomplete packet received. Expected %d, got %d.", remaining_bytes, bytes_read);
        uart_flush_input(CURRENT_SENSOR_PORT);
        return false;
    }

    UARTPacket* packet = reinterpret_cast<UARTPacket*>(buffer);

    // 3. Verify EOF and Checksum
    if (packet->eof != EOF_BYTE) {
        ESP_LOGW(TAG, "Invalid EOF byte. Expected 0x%X, got 0x%X.", EOF_BYTE, packet->eof);
        return false;
    }

    uint8_t calculated_checksum = calculateChecksum(packet->data);
    if (packet->checksum != calculated_checksum) {
        ESP_LOGW(TAG, "Checksum mismatch. Expected %u, got %u.", calculated_checksum, packet->checksum);
        return false;
    }

    // 4. Data is valid, update sensor data and filters
    sensor_data = packet->data;
    current1_filter.add(sensor_data.current1);
    current2_filter.add(sensor_data.current2);
    voltage_filter.add(sensor_data.voltage1);
    
    data_valid = true;
    last_update_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "Valid packet received: I1=%.2fA, I2=%.2fA, V=%.1fV", sensor_data.current1, sensor_data.current2, sensor_data.voltage1);
    
    return true;
}

void UARTCurrentSensor::sendIPAddress(const String& ipAddress) {
    const char* TAG = "UARTCurrentSensor::sendIPAddress";
    if (ipAddress.length() == 0 || ipAddress == "0.0.0.0") {
        ESP_LOGW(TAG, "Invalid IP address provided: %s", ipAddress.c_str());
        return;
    }
    String ipCommand = "I" + ipAddress + "\n";
    uart_write_bytes(CURRENT_SENSOR_PORT, ipCommand.c_str(), ipCommand.length());
    vTaskDelay(pdMS_TO_TICKS(100));
}

void UARTCurrentSensor::sendWiFiMode(bool isAPMode, const String& ipAddress) {
    const char* TAG = "UARTCurrentSensor::sendWiFiMode";
    String modeCommand = "M";
    modeCommand += isAPMode ? "A" : "S";
    modeCommand += ipAddress + "\n";
    uart_write_bytes(CURRENT_SENSOR_PORT, modeCommand.c_str(), modeCommand.length());
    vTaskDelay(pdMS_TO_TICKS(100));
}

void UARTCurrentSensor::sendWiFiStrength(int rssi) {
    const char* TAG = "UARTCurrentSensor::sendWiFiStrength";
    
    // Convert RSSI to 0-4 scale
    int strength = 0;
    if (rssi >= -50) strength = 4;      // Excellent
    else if (rssi >= -60) strength = 3; // Good  
    else if (rssi >= -70) strength = 2; // Fair
    else if (rssi >= -80) strength = 1; // Poor
    else strength = 0;                  // Very poor/disconnected
    
    String strengthCommand = "W" + String(strength) + "\n";
    uart_write_bytes(CURRENT_SENSOR_PORT, strengthCommand.c_str(), strengthCommand.length());
    vTaskDelay(pdMS_TO_TICKS(50));
    
    ESP_LOGD(TAG, "Sent WiFi strength: RSSI=%d, Strength=%d", rssi, strength);
}

void UARTCurrentSensor::uartCurrentSensorTask(void *pvParameters) {
    UARTCurrentSensor *sensor = static_cast<UARTCurrentSensor*>(pvParameters);
    const char* TAG = "UARTCurrentSensorTask";
    
    ESP_LOGI(TAG, "UART Current Sensor task started on UART port %d (GPIO %d/%d)", 
             CURRENT_SENSOR_PORT, CURRENT_SENSOR_TXD, CURRENT_SENSOR_RXD);
    
    uint32_t failedAttempts = 0;
    
    while (1) {
        ESP_LOGD(TAG, "Requesting data from Arduino Nano...");
        sensor->requestData();
        vTaskDelay(pdMS_TO_TICKS(20)); // Short delay for the command to be sent

        if (!sensor->receiveData()) {
            failedAttempts++;
            if (failedAttempts % 10 == 0) { // Log every 10th failure
                ESP_LOGW(TAG, "Failed to receive data from Arduino (attempts: %u)", failedAttempts);
            }
            
            // If data is not received, maybe the sensor is offline
            if (sensor->data_valid && (xTaskGetTickCount() * portTICK_PERIOD_MS - sensor->last_update_time) > 2000) {
                ESP_LOGW(TAG, "Sensor data timeout. Marking as invalid.");
                sensor->data_valid = false;
            }
        } else {
            if (failedAttempts > 0) {
                ESP_LOGI(TAG, "Arduino communication restored after %u failed attempts", failedAttempts);
                failedAttempts = 0;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(150)); // Adjust loop rate
    }
}
