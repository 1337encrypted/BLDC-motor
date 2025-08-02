#pragma once
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

// Door identifiers
enum class DoorID : uint8_t {
    FRONT_LEFT = 1,
    FRONT_RIGHT = 2,
    REAR_LEFT = 3,
    REAR_RIGHT = 4,
    ALL_DOORS = 0xFF
};

class DoorLock {
    private:
    gpio_num_t door1Pin, door2Pin, door3Pin, door4Pin;    // 4 solenoid pins
    bool door1State, door2State, door3State, door4State; // Individual door states
    
    // IRF520N MOSFET control (HIGH = activate solenoid = lock)
    void controlSolenoid(gpio_num_t pin, bool activate);
    
    public:
    DoorLock(gpio_num_t door1 = GPIO_NUM_NC, gpio_num_t door2 = GPIO_NUM_NC, 
             gpio_num_t door3 = GPIO_NUM_NC, gpio_num_t door4 = GPIO_NUM_NC);
    void begin();
    
    // Individual door control
    void lockDoor(DoorID doorId);
    void unlockDoor(DoorID doorId);
    void toggleDoor(DoorID doorId);
    
    // All doors control
    void lockAllDoors();
    void unlockAllDoors();
    void toggleAllDoors();
    
    // Status methods
    bool isDoorLocked(DoorID doorId) const;
    bool areAllDoorsLocked() const;
    uint8_t getLockedDoorCount() const;
    
    // Advanced control
    void lockFrontDoors();
    void unlockFrontDoors();
    void lockRearDoors();
    void unlockRearDoors();
    
    // Safety features
    void emergencyUnlock();
    void setAutoLockDelay(uint32_t delayMs);
    
    // Status reporting
    void printStatus() const;
    String getStatusString() const;
};

DoorLock::DoorLock(gpio_num_t door1, gpio_num_t door2, gpio_num_t door3, gpio_num_t door4) :
door1Pin(door1), door2Pin(door2), door3Pin(door3), door4Pin(door4),
door1State(false), door2State(false), door3State(false), door4State(false)
{}

void DoorLock::begin() {
    const char* TAG = "DoorLock::begin";
    
    // Configure all door pins as outputs
    gpio_num_t pins[] = {door1Pin, door2Pin, door3Pin, door4Pin};
    
    for (int i = 0; i < 4; i++) {
        if (pins[i] != GPIO_NUM_NC) {
            gpio_config_t gpioConfig = {
                .pin_bit_mask = (1ULL << pins[i]),
                .mode = GPIO_MODE_OUTPUT,
                .pull_up_en = GPIO_PULLUP_DISABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE
            };
            ESP_ERROR_CHECK(gpio_config(&gpioConfig));
            
            // Initialize to unlocked state (LOW = solenoid off = unlocked)
            gpio_set_level(pins[i], 0);
        }
    }
    
    // Initialize all doors to unlocked state
    door1State = door2State = door3State = door4State = false;
    
    ESP_LOGI(TAG, "4-Door lock system initialized - all doors unlocked");
}

void DoorLock::controlSolenoid(gpio_num_t pin, bool activate) {
    if (pin != GPIO_NUM_NC) {
        // IRF520N MOSFET: HIGH = activate solenoid = lock door
        gpio_set_level(pin, activate ? 1 : 0);
    }
}

void DoorLock::lockDoor(DoorID doorId) {
    const char* TAG = "DoorLock::lockDoor";
    
    switch (doorId) {
        case DoorID::FRONT_LEFT:
            controlSolenoid(door1Pin, true);
            door1State = true;
            ESP_LOGI(TAG, "Front Left door locked");
            break;
            
        case DoorID::FRONT_RIGHT:
            controlSolenoid(door2Pin, true);
            door2State = true;
            ESP_LOGI(TAG, "Front Right door locked");
            break;
            
        case DoorID::REAR_LEFT:
            controlSolenoid(door3Pin, true);
            door3State = true;
            ESP_LOGI(TAG, "Rear Left door locked");
            break;
            
        case DoorID::REAR_RIGHT:
            controlSolenoid(door4Pin, true);
            door4State = true;
            ESP_LOGI(TAG, "Rear Right door locked");
            break;
            
        case DoorID::ALL_DOORS:
            lockAllDoors();
            break;
    }
}

void DoorLock::unlockDoor(DoorID doorId) {
    const char* TAG = "DoorLock::unlockDoor";
    
    switch (doorId) {
        case DoorID::FRONT_LEFT:
            controlSolenoid(door1Pin, false);
            door1State = false;
            ESP_LOGI(TAG, "Front Left door unlocked");
            break;
            
        case DoorID::FRONT_RIGHT:
            controlSolenoid(door2Pin, false);
            door2State = false;
            ESP_LOGI(TAG, "Front Right door unlocked");
            break;
            
        case DoorID::REAR_LEFT:
            controlSolenoid(door3Pin, false);
            door3State = false;
            ESP_LOGI(TAG, "Rear Left door unlocked");
            break;
            
        case DoorID::REAR_RIGHT:
            controlSolenoid(door4Pin, false);
            door4State = false;
            ESP_LOGI(TAG, "Rear Right door unlocked");
            break;
            
        case DoorID::ALL_DOORS:
            unlockAllDoors();
            break;
    }
}

void DoorLock::toggleDoor(DoorID doorId) {
    if (isDoorLocked(doorId)) {
        unlockDoor(doorId);
    } else {
        lockDoor(doorId);
    }
}

void DoorLock::lockAllDoors() {
    controlSolenoid(door1Pin, true);
    controlSolenoid(door2Pin, true);
    controlSolenoid(door3Pin, true);
    controlSolenoid(door4Pin, true);
    
    door1State = door2State = door3State = door4State = true;
    
    ESP_LOGI("DoorLock", "All doors locked");
}

void DoorLock::unlockAllDoors() {
    controlSolenoid(door1Pin, false);
    controlSolenoid(door2Pin, false);
    controlSolenoid(door3Pin, false);
    controlSolenoid(door4Pin, false);
    
    door1State = door2State = door3State = door4State = false;
    
    ESP_LOGI("DoorLock", "All doors unlocked");
}

void DoorLock::toggleAllDoors() {
    if (areAllDoorsLocked()) {
        unlockAllDoors();
    } else {
        lockAllDoors();
    }
}

bool DoorLock::isDoorLocked(DoorID doorId) const {
    switch (doorId) {
        case DoorID::FRONT_LEFT: return door1State;
        case DoorID::FRONT_RIGHT: return door2State;
        case DoorID::REAR_LEFT: return door3State;
        case DoorID::REAR_RIGHT: return door4State;
        case DoorID::ALL_DOORS: return areAllDoorsLocked();
        default: return false;
    }
}

bool DoorLock::areAllDoorsLocked() const {
    return door1State && door2State && door3State && door4State;
}

uint8_t DoorLock::getLockedDoorCount() const {
    return (door1State ? 1 : 0) + (door2State ? 1 : 0) + 
           (door3State ? 1 : 0) + (door4State ? 1 : 0);
}

void DoorLock::lockFrontDoors() {
    lockDoor(DoorID::FRONT_LEFT);
    lockDoor(DoorID::FRONT_RIGHT);
    ESP_LOGI("DoorLock", "Front doors locked");
}

void DoorLock::unlockFrontDoors() {
    unlockDoor(DoorID::FRONT_LEFT);
    unlockDoor(DoorID::FRONT_RIGHT);
    ESP_LOGI("DoorLock", "Front doors unlocked");
}

void DoorLock::lockRearDoors() {
    lockDoor(DoorID::REAR_LEFT);
    lockDoor(DoorID::REAR_RIGHT);
    ESP_LOGI("DoorLock", "Rear doors locked");
}

void DoorLock::unlockRearDoors() {
    unlockDoor(DoorID::REAR_LEFT);
    unlockDoor(DoorID::REAR_RIGHT);
    ESP_LOGI("DoorLock", "Rear doors unlocked");
}

void DoorLock::emergencyUnlock() {
    ESP_LOGW("DoorLock", "EMERGENCY UNLOCK - All doors unlocked");
    unlockAllDoors();
}

void DoorLock::setAutoLockDelay(uint32_t delayMs) {
    // Future implementation for auto-lock timer
    ESP_LOGI("DoorLock", "Auto-lock delay set to %u ms", delayMs);
}

void DoorLock::printStatus() const {
    ESP_LOGI("DoorLock", "Door Status - FL:%s FR:%s RL:%s RR:%s (%u/4 locked)", 
             door1State ? "L" : "U", door2State ? "L" : "U",
             door3State ? "L" : "U", door4State ? "L" : "U",
             getLockedDoorCount());
}

String DoorLock::getStatusString() const {
    return String("FL:") + (door1State ? "L" : "U") + 
           " FR:" + (door2State ? "L" : "U") +
           " RL:" + (door3State ? "L" : "U") + 
           " RR:" + (door4State ? "L" : "U");
}