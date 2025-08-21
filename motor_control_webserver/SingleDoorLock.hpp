#pragma once
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

// Door positions for identification
enum class DoorPosition : uint8_t {
    FRONT_LEFT = 1,
    FRONT_RIGHT = 2,
    REAR_LEFT = 3,
    REAR_RIGHT = 4,
    CUSTOM = 99
};

class SingleDoorLock {
private:
    gpio_num_t solenoidPin;
    bool isLocked;
    bool isInitialized;
    DoorPosition position;
    String doorName;
    uint32_t lockTime;
    uint32_t unlockTime;
    
    // IRF520N MOSFET control (HIGH = activate solenoid = lock)
    void controlSolenoid(bool activate);
    
public:
    SingleDoorLock(gpio_num_t pin = GPIO_NUM_NC, DoorPosition pos = DoorPosition::CUSTOM, 
                   const String& name = "Door");
    ~SingleDoorLock();
    
    // Core functionality
    void begin();
    void end();
    
    // Lock control
    void lock();
    void unlock();
    void toggle();
    void setState(bool locked);
    
    // Status methods
    bool isLockLocked() const { return isLocked; }
    bool isUnlocked() const { return !isLocked; }
    DoorPosition getPosition() const { return position; }
    gpio_num_t getPin() const { return solenoidPin; }
    String getName() const { return doorName; }
    uint32_t getLockTime() const { return lockTime; }
    uint32_t getUnlockTime() const { return unlockTime; }
    
    // Advanced features
    void setLockTiming(uint32_t lockMs = 500, uint32_t unlockMs = 500);
    void pulseLock(uint16_t pulseTime = 100); // Short pulse to activate lock
    void pulseUnlock(uint16_t pulseTime = 100); // Short pulse to deactivate lock
    
    // Safety features
    void emergencyUnlock();
    void forceUnlock(); // Bypass all safety checks
    void setAutoLockDelay(uint32_t delayMs); // Future implementation
    
    // Status reporting
    void printStatus() const;
    String getStatusString() const;
    String getPositionString() const;
    
    // Diagnostics
    bool testSolenoid(uint16_t testTime = 200); // Test solenoid operation
    uint32_t getOperationCount() const; // Future implementation
    
private:
    uint32_t operationCount;
    uint32_t lastOperationTime;
};

SingleDoorLock::SingleDoorLock(gpio_num_t pin, DoorPosition pos, const String& name) 
    : solenoidPin(pin), isLocked(false), isInitialized(false), position(pos), 
      doorName(name), lockTime(500), unlockTime(500), operationCount(0), lastOperationTime(0) {
}

SingleDoorLock::~SingleDoorLock() {
    end();
}

void SingleDoorLock::begin() {
    if (isInitialized) return;
    
    const char* TAG = "SingleDoorLock::begin";
    
    if (solenoidPin == GPIO_NUM_NC) {
        ESP_LOGW(TAG, "%s: Invalid pin configuration", doorName.c_str());
        return;
    }
    
    // Configure GPIO pin as output
    gpio_config_t gpioConfig = {
        .pin_bit_mask = (1ULL << solenoidPin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t result = gpio_config(&gpioConfig);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "%s: Failed to configure GPIO %d", doorName.c_str(), solenoidPin);
        return;
    }
    
    // Initialize to unlocked state (LOW = solenoid off = unlocked)
    gpio_set_level(solenoidPin, 0);
    isLocked = false;
    isInitialized = true;
    
    ESP_LOGI(TAG, "%s (%s) initialized on GPIO %d - UNLOCKED", 
             doorName.c_str(), getPositionString().c_str(), solenoidPin);
}

void SingleDoorLock::end() {
    if (isInitialized) {
        unlock();
        isInitialized = false;
        ESP_LOGI("SingleDoorLock", "%s deinitialized", doorName.c_str());
    }
}

void SingleDoorLock::controlSolenoid(bool activate) {
    if (isInitialized && solenoidPin != GPIO_NUM_NC) {
        // IRF520N MOSFET: HIGH = gate on = MOSFET on = solenoid on = door locked
        gpio_set_level(solenoidPin, activate ? 1 : 0);
        lastOperationTime = millis();
        operationCount++;
    }
}

void SingleDoorLock::lock() {
    if (!isInitialized) {
        ESP_LOGW("SingleDoorLock", "%s: Not initialized", doorName.c_str());
        return;
    }
    
    if (isLocked) {
        ESP_LOGD("SingleDoorLock", "%s: Already locked", doorName.c_str());
        return;
    }
    
    controlSolenoid(true);
    isLocked = true;
    ESP_LOGI("SingleDoorLock", "%s LOCKED", doorName.c_str());
}

void SingleDoorLock::unlock() {
    if (!isInitialized) {
        ESP_LOGW("SingleDoorLock", "%s: Not initialized", doorName.c_str());
        return;
    }
    
    if (!isLocked) {
        ESP_LOGD("SingleDoorLock", "%s: Already unlocked", doorName.c_str());
        return;
    }
    
    controlSolenoid(false);
    isLocked = false;
    ESP_LOGI("SingleDoorLock", "%s UNLOCKED", doorName.c_str());
}

void SingleDoorLock::toggle() {
    if (isLocked) {
        unlock();
    } else {
        lock();
    }
}

void SingleDoorLock::setState(bool locked) {
    if (locked) {
        lock();
    } else {
        unlock();
    }
}

void SingleDoorLock::setLockTiming(uint32_t lockMs, uint32_t unlockMs) {
    lockTime = lockMs;
    unlockTime = unlockMs;
    ESP_LOGI("SingleDoorLock", "%s timing set: lock=%ums, unlock=%ums", 
             doorName.c_str(), lockTime, unlockTime);
}

void SingleDoorLock::pulseLock(uint16_t pulseTime) {
    if (!isInitialized) return;
    
    ESP_LOGI("SingleDoorLock", "%s pulse lock %dms", doorName.c_str(), pulseTime);
    controlSolenoid(true);
    vTaskDelay(pdMS_TO_TICKS(pulseTime));
    controlSolenoid(false);
    isLocked = true; // Assume lock engaged after pulse
}

void SingleDoorLock::pulseUnlock(uint16_t pulseTime) {
    if (!isInitialized) return;
    
    ESP_LOGI("SingleDoorLock", "%s pulse unlock %dms", doorName.c_str(), pulseTime);
    controlSolenoid(false);
    vTaskDelay(pdMS_TO_TICKS(pulseTime));
    isLocked = false; // Assume lock disengaged after pulse
}

void SingleDoorLock::emergencyUnlock() {
    ESP_LOGW("SingleDoorLock", "%s EMERGENCY UNLOCK", doorName.c_str());
    controlSolenoid(false);
    isLocked = false;
}

void SingleDoorLock::forceUnlock() {
    ESP_LOGW("SingleDoorLock", "%s FORCE UNLOCK (bypassing safety)", doorName.c_str());
    controlSolenoid(false);
    isLocked = false;
}

void SingleDoorLock::setAutoLockDelay(uint32_t delayMs) {
    // Future implementation with timer
    ESP_LOGI("SingleDoorLock", "%s auto-lock delay set to %u ms", doorName.c_str(), delayMs);
}

bool SingleDoorLock::testSolenoid(uint16_t testTime) {
    if (!isInitialized) return false;
    
    ESP_LOGI("SingleDoorLock", "%s testing solenoid for %dms", doorName.c_str(), testTime);
    
    bool originalState = isLocked;
    
    // Test activation
    controlSolenoid(true);
    vTaskDelay(pdMS_TO_TICKS(testTime));
    controlSolenoid(false);
    
    // Restore original state
    setState(originalState);
    
    ESP_LOGI("SingleDoorLock", "%s solenoid test completed", doorName.c_str());
    return true;
}

uint32_t SingleDoorLock::getOperationCount() const {
    return operationCount;
}

void SingleDoorLock::printStatus() const {
    ESP_LOGI("SingleDoorLock", "%s (%s) Status - Pin: %d, State: %s, Operations: %u", 
             doorName.c_str(), getPositionString().c_str(), solenoidPin, 
             isLocked ? "LOCKED" : "UNLOCKED", operationCount);
}

String SingleDoorLock::getStatusString() const {
    return doorName + ":" + (isLocked ? "LOCKED" : "UNLOCKED") + " (GPIO" + String(solenoidPin) + ")";
}

String SingleDoorLock::getPositionString() const {
    switch (position) {
        case DoorPosition::FRONT_LEFT: return "Front Left";
        case DoorPosition::FRONT_RIGHT: return "Front Right";
        case DoorPosition::REAR_LEFT: return "Rear Left";
        case DoorPosition::REAR_RIGHT: return "Rear Right";
        case DoorPosition::CUSTOM: return "Custom";
        default: return "Unknown";
    }
}