#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ArduinoJson.h"
#include "esp_log.h"

// Forward declarations from GLOBALS.hpp
extern PwmGenerator motorPWM1;
extern PwmGenerator motorPWM2;
extern BLDCPulseCalculator motorPulse1;
extern BLDCPulseCalculator motorPulse2;
extern DoorLock doorLock;
extern UARTCurrentSensor currentSensor;

// Global state variables
extern bool doorsLocked;
extern bool lightsOn;
extern uint8_t currentPWM;
extern float motor1Speed;
extern float motor2Speed;
extern float motor1Current;
extern float motor2Current;
extern float systemVoltage;

class DataCollector {
private:
    TaskHandle_t taskHandle;
    bool initialized;
    unsigned long lastCollectionTime;
    const BaseType_t app_cpu;
    
    // Data collection methods
    void collectMotorData();
    void collectCurrentData();
    void collectSystemData();
    
public:
    DataCollector(const BaseType_t core = 1);
    ~DataCollector();
    
    // Core functionality
    void begin();
    void end();
    bool isRunning() const { return initialized; }
    
    // Data access methods
    DynamicJsonDocument getMotorData();
    DynamicJsonDocument getCurrentData();
    DynamicJsonDocument getSystemData();
    DynamicJsonDocument getAllData();
    
    // Status methods
    float getMotor1Speed() const { return motor1Speed; }
    float getMotor2Speed() const { return motor2Speed; }
    float getMotor1Current() const { return motor1Current; }
    float getMotor2Current() const { return motor2Current; }
    float getSystemVoltage() const { return systemVoltage; }
    uint8_t getCurrentPWM() const { return currentPWM; }
    bool getDoorsLocked() const { return doorsLocked; }
    bool getLightsOn() const { return lightsOn; }
    
    // Data update rate control
    void setUpdateRate(uint32_t rateMs);
    uint32_t getUpdateRate() const;
    
    // FreeRTOS task
    static void dataCollectorTask(void* pvParameters);
};

DataCollector::DataCollector(const BaseType_t core) 
    : taskHandle(nullptr), initialized(false), lastCollectionTime(0), app_cpu(core) {
}

DataCollector::~DataCollector() {
    end();
}

void DataCollector::begin() {
    if (initialized) return;
    
    const char* TAG = "DataCollector::begin";
    
    BaseType_t result = xTaskCreatePinnedToCore(
        &dataCollectorTask,
        "data_collector_task",
        4096,
        this,
        1,
        &taskHandle,
        app_cpu
    );
    
    if (result == pdPASS) {
        initialized = true;
        ESP_LOGI(TAG, "Data collector task created successfully on core %d", app_cpu);
    } else {
        ESP_LOGE(TAG, "Failed to create data collector task");
    }
}

void DataCollector::end() {
    if (initialized && taskHandle != nullptr) {
        vTaskDelete(taskHandle);
        taskHandle = nullptr;
        initialized = false;
        ESP_LOGI("DataCollector", "Data collector task stopped");
    }
}

void DataCollector::collectMotorData() {
    // Update motor speeds from pulse calculators
    motor1Speed = motorPulse1.getSpeed();
    motor2Speed = motorPulse2.getSpeed();
    
    // Get current PWM value
    currentPWM = motorPWM1.getPwm(); // Assuming synchronized PWM
    
    ESP_LOGD("DataCollector", "Motor speeds: M1=%.1f RPM, M2=%.1f RPM, PWM=%d", 
             motor1Speed, motor2Speed, currentPWM);
}

void DataCollector::collectCurrentData() {
    if (currentSensor.isDataValid()) {
        motor1Current = currentSensor.getCurrent1();
        motor2Current = currentSensor.getCurrent2();
        systemVoltage = currentSensor.getVoltage();
        
        ESP_LOGD("DataCollector", "Current data: I1=%.2fA, I2=%.2fA, V=%.1fV", 
                 motor1Current, motor2Current, systemVoltage);
    } else {
        ESP_LOGW("DataCollector", "Current sensor data invalid");
        // Keep last known values or set to 0
        motor1Current = 0.0f;
        motor2Current = 0.0f;
        systemVoltage = 0.0f;
    }
}

void DataCollector::collectSystemData() {
    // Update control states
    doorsLocked = doorLock.areAllDoorsLocked();
    // lightsOn state managed by control interface
    
    ESP_LOGD("DataCollector", "System state: Doors=%s, Lights=%s", 
             doorsLocked ? "locked" : "unlocked", lightsOn ? "on" : "off");
}

DynamicJsonDocument DataCollector::getMotorData() {
    DynamicJsonDocument doc(1024);
    
    doc["motor1"]["speed"] = motor1Speed;
    doc["motor1"]["current"] = motor1Current;
    doc["motor2"]["speed"] = motor2Speed;
    doc["motor2"]["current"] = motor2Current;
    doc["pwm"] = currentPWM;
    doc["timestamp"] = millis();
    
    return doc;
}

DynamicJsonDocument DataCollector::getCurrentData() {
    DynamicJsonDocument doc(1024);
    
    doc["current1"] = motor1Current;
    doc["current2"] = motor2Current;
    doc["voltage"] = systemVoltage;
    doc["power1"] = motor1Current * systemVoltage;
    doc["power2"] = motor2Current * systemVoltage;
    doc["timestamp"] = millis();
    
    return doc;
}

DynamicJsonDocument DataCollector::getSystemData() {
    DynamicJsonDocument doc(1024);
    
    doc["heap"] = ESP.getFreeHeap();
    doc["uptime"] = millis();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["doors_locked"] = doorsLocked;
    doc["lights_on"] = lightsOn;
    doc["current_sensor_valid"] = currentSensor.isDataValid();
    doc["timestamp"] = millis();
    
    return doc;
}

DynamicJsonDocument DataCollector::getAllData() {
    DynamicJsonDocument doc(2048);
    
    // Motor data
    JsonObject motor1 = doc["motor1"].to<JsonObject>();
    motor1["speed"] = motor1Speed;
    motor1["current"] = motor1Current;
    motor1["power"] = motor1Current * systemVoltage;
    
    JsonObject motor2 = doc["motor2"].to<JsonObject>();
    motor2["speed"] = motor2Speed;
    motor2["current"] = motor2Current;
    motor2["power"] = motor2Current * systemVoltage;
    
    // Control data
    doc["pwm"] = currentPWM;
    doc["voltage"] = systemVoltage;
    doc["doors_locked"] = doorsLocked;
    doc["lights_on"] = lightsOn;
    
    // System data
    doc["heap"] = ESP.getFreeHeap();
    doc["uptime"] = millis();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["current_sensor_valid"] = currentSensor.isDataValid();
    doc["timestamp"] = millis();
    
    return doc;
}

void DataCollector::setUpdateRate(uint32_t rateMs) {
    // Implementation would modify task delay
    ESP_LOGI("DataCollector", "Update rate set to %u ms", rateMs);
}

uint32_t DataCollector::getUpdateRate() const {
    return 100; // Default 100ms (10Hz)
}

void DataCollector::dataCollectorTask(void* pvParameters) {
    DataCollector* collector = static_cast<DataCollector*>(pvParameters);
    const char* TAG = "DataCollectorTask";
    
    ESP_LOGI(TAG, "Data collector task started");
    
    while (true) {
        try {
            // Collect all sensor data
            collector->collectMotorData();
            collector->collectCurrentData();
            collector->collectSystemData();
            
            collector->lastCollectionTime = millis();
            
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Error in data collection: %s", e.what());
        }
        
        // 10Hz update rate (100ms delay)
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}