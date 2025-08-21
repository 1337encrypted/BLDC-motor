#pragma once
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

class Light {
private:
    gpio_num_t controlPin;
    bool currentState;
    bool isInitialized;
    String lightName;
    
    // IRF520N MOSFET control (HIGH = light on, LOW = light off)
    void controlMOSFET(bool activate);
    
public:
    Light(gpio_num_t pin = GPIO_NUM_NC, const String& name = "Light");
    ~Light();
    
    // Core functionality
    void begin();
    void end();
    
    // Light control
    void turnOn();
    void turnOff();
    void toggle();
    void setState(bool state);
    
    // Status methods
    bool isOn() const { return currentState; }
    bool isOff() const { return !currentState; }
    gpio_num_t getPin() const { return controlPin; }
    String getName() const { return lightName; }
    
    // Advanced features
    void blink(uint16_t onTime = 500, uint16_t offTime = 500, uint8_t cycles = 1);
    void fade(uint16_t fadeTime = 1000); // Future PWM implementation
    
    // Status reporting
    void printStatus() const;
    String getStatusString() const;
    
    // Safety features
    void emergencyOff();
    void setAutoOffDelay(uint32_t delayMs); // Future implementation
};

Light::Light(gpio_num_t pin, const String& name) 
    : controlPin(pin), currentState(false), isInitialized(false), lightName(name) {
}

Light::~Light() {
    end();
}

void Light::begin() {
    if (isInitialized) return;
    
    const char* TAG = "Light::begin";
    
    if (controlPin == GPIO_NUM_NC) {
        ESP_LOGW(TAG, "%s: Invalid pin configuration", lightName.c_str());
        return;
    }
    
    // Configure GPIO pin as output
    gpio_config_t gpioConfig = {
        .pin_bit_mask = (1ULL << controlPin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t result = gpio_config(&gpioConfig);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "%s: Failed to configure GPIO %d", lightName.c_str(), controlPin);
        return;
    }
    
    // Initialize to off state (LOW = MOSFET off = light off)
    gpio_set_level(controlPin, 0);
    currentState = false;
    isInitialized = true;
    
    ESP_LOGI(TAG, "%s initialized on GPIO %d - OFF", lightName.c_str(), controlPin);
}

void Light::end() {
    if (isInitialized) {
        turnOff();
        isInitialized = false;
        ESP_LOGI("Light", "%s deinitialized", lightName.c_str());
    }
}

void Light::controlMOSFET(bool activate) {
    if (isInitialized && controlPin != GPIO_NUM_NC) {
        // IRF520N MOSFET: HIGH = gate on = MOSFET on = light on
        gpio_set_level(controlPin, activate ? 1 : 0);
    }
}

void Light::turnOn() {
    if (!isInitialized) {
        ESP_LOGW("Light", "%s: Not initialized", lightName.c_str());
        return;
    }
    
    controlMOSFET(true);
    currentState = true;
    ESP_LOGI("Light", "%s turned ON", lightName.c_str());
}

void Light::turnOff() {
    if (!isInitialized) {
        ESP_LOGW("Light", "%s: Not initialized", lightName.c_str());
        return;
    }
    
    controlMOSFET(false);
    currentState = false;
    ESP_LOGI("Light", "%s turned OFF", lightName.c_str());
}

void Light::toggle() {
    if (currentState) {
        turnOff();
    } else {
        turnOn();
    }
}

void Light::setState(bool state) {
    if (state) {
        turnOn();
    } else {
        turnOff();
    }
}

void Light::blink(uint16_t onTime, uint16_t offTime, uint8_t cycles) {
    if (!isInitialized) return;
    
    ESP_LOGI("Light", "%s blinking %d cycles", lightName.c_str(), cycles);
    
    bool originalState = currentState;
    
    for (uint8_t i = 0; i < cycles; i++) {
        turnOn();
        vTaskDelay(pdMS_TO_TICKS(onTime));
        turnOff();
        if (i < cycles - 1) { // Don't delay after last cycle
            vTaskDelay(pdMS_TO_TICKS(offTime));
        }
    }
    
    // Restore original state
    setState(originalState);
}

void Light::fade(uint16_t fadeTime) {
    // Future implementation with PWM
    ESP_LOGI("Light", "%s fade effect (PWM implementation needed)", lightName.c_str());
    toggle(); // For now, just toggle
}

void Light::emergencyOff() {
    ESP_LOGW("Light", "%s EMERGENCY OFF", lightName.c_str());
    controlMOSFET(false);
    currentState = false;
}

void Light::setAutoOffDelay(uint32_t delayMs) {
    // Future implementation with timer
    ESP_LOGI("Light", "%s auto-off delay set to %u ms", lightName.c_str(), delayMs);
}

void Light::printStatus() const {
    ESP_LOGI("Light", "%s Status - Pin: %d, State: %s", 
             lightName.c_str(), controlPin, currentState ? "ON" : "OFF");
}

String Light::getStatusString() const {
    return lightName + ":" + (currentState ? "ON" : "OFF") + " (GPIO" + String(controlPin) + ")";
}