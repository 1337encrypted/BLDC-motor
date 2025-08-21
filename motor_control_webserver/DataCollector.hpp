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
extern UARTCurrentSensor currentSensor;

// New modular objects
extern Light leftLight;
extern Light rightLight;
extern SingleDoorLock frontLeftDoor;
extern SingleDoorLock frontRightDoor;
extern SingleDoorLock rearLeftDoor;
extern SingleDoorLock rearRightDoor;

// Global state variables
extern bool doorsLocked;
extern bool door1Locked;
extern bool door2Locked;
extern bool door3Locked;
extern bool door4Locked;
extern bool leftLightOn;
extern bool rightLightOn;
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
    bool getLightsOn() const { return leftLightOn || rightLightOn; }
    
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
    // Update individual door states
    door1Locked = frontLeftDoor.isLockLocked();
    door2Locked = frontRightDoor.isLockLocked();
    door3Locked = rearLeftDoor.isLockLocked();
    door4Locked = rearRightDoor.isLockLocked();
    doorsLocked = door1Locked && door2Locked && door3Locked && door4Locked;
    
    // Update individual light states
    leftLightOn = leftLight.isOn();
    rightLightOn = rightLight.isOn();
    
    ESP_LOGD("DataCollector", "System state: Doors FL:%s FR:%s RL:%s RR:%s, Lights L:%s R:%s", 
             door1Locked ? "L" : "U", door2Locked ? "L" : "U",
             door3Locked ? "L" : "U", door4Locked ? "L" : "U",
             leftLightOn ? "ON" : "OFF", rightLightOn ? "ON" : "OFF");
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
    doc["lights_on"] = leftLightOn || rightLightOn;
    doc["current_sensor_valid"] = currentSensor.isDataValid();
    doc["timestamp"] = millis();
    
    return doc;
}

DynamicJsonDocument DataCollector::getAllData() {
    DynamicJsonDocument doc(3072); // Increased size for additional data
    
    // Motor data (now includes 4 motors for UI compatibility)
    JsonObject motor1 = doc["motor1"].to<JsonObject>();
    motor1["speed"] = motor1Speed;
    motor1["current"] = motor1Current;
    motor1["power"] = motor1Current * systemVoltage;
    
    JsonObject motor2 = doc["motor2"].to<JsonObject>();
    motor2["speed"] = motor2Speed;
    motor2["current"] = motor2Current;
    motor2["power"] = motor2Current * systemVoltage;
    
    // Add motor3 and motor4 for UI compatibility (currently using same data)
    JsonObject motor3 = doc["motor3"].to<JsonObject>();
    motor3["speed"] = 0; // Not implemented yet
    motor3["current"] = 0.0f;
    motor3["power"] = 0.0f;
    
    JsonObject motor4 = doc["motor4"].to<JsonObject>();
    motor4["speed"] = 0; // Not implemented yet
    motor4["current"] = 0.0f;
    motor4["power"] = 0.0f;
    
    // Control data
    doc["pwm"] = currentPWM;
    doc["voltage"] = systemVoltage;
    
    // Individual door states (matching UI expectations)
    JsonObject doors = doc["doors"].to<JsonObject>();
    doors["left_locked"] = door1Locked;   // Front left door mapped to "left"
    doors["right_locked"] = door2Locked; // Front right door mapped to "right"
    doors["all_locked"] = doorsLocked;
    doors["front_left"] = door1Locked;
    doors["front_right"] = door2Locked;
    doors["rear_left"] = door3Locked;
    doors["rear_right"] = door4Locked;
    
    // Individual light states (matching UI expectations)
    JsonObject lights = doc["lights"].to<JsonObject>();
    lights["left_on"] = leftLightOn;
    lights["right_on"] = rightLightOn;
    lights["all_on"] = leftLightOn || rightLightOn; // Combined state
    
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