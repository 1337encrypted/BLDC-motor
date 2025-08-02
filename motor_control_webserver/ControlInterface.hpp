#pragma once

#include "ArduinoJson.h"
#include "esp_log.h"
#include "driver/gpio.h"

// WebSocket functionality removed - control is now handled by HTTP API

// Forward declarations from GLOBALS.hpp
extern PwmGenerator motorPWM1;
extern PwmGenerator motorPWM2;
extern DoorLock doorLock;

// Global state variables
extern bool doorsLocked;
extern bool lightsOn;
extern uint8_t currentPWM;
extern gpio_num_t lightsPin;

class ControlInterface {
private:
    bool initialized;
    uint8_t maxPWM;
    uint8_t minPWM;
    
    // Safety limits
    bool safetyEnabled;
    float maxCurrent;
    uint8_t emergencyStopPWM;
    
    // Control validation
    bool validatePWMValue(uint8_t pwm);
    bool validateCommand(const String& command);
    
    // Hardware control methods
    void controlLights(bool state);
    void controlDoors(bool lock);
    void controlMotorPWM(uint8_t pwm);
    
    // Safety methods
    bool checkSafetyLimits();
    void emergencyStop();
    
public:
    ControlInterface();
    ~ControlInterface();
    
    // Core functionality
    void begin();
    void end();
    bool isInitialized() const { return initialized; }
    
    // Command processing - simplified for HTTP API
    bool processCommand(const String& command, const JsonDocument& data);
    
    // Direct control methods
    bool setPWM(uint8_t pwm);
    bool toggleDoors();
    bool toggleLights();
    bool lockDoors();
    bool unlockDoors();
    bool setLights(bool state);
    
    // Safety configuration
    void setSafetyLimits(float maxCurrentAmps, uint8_t maxPwmValue);
    void enableSafety(bool enable);
    bool isSafetyEnabled() const { return safetyEnabled; }
    
    // Status methods
    uint8_t getCurrentPWM() const { return currentPWM; }
    bool areDoorsLocked() const { return doorsLocked; }
    bool areLightsOn() const { return lightsOn; }
    
    // Response methods removed - HTTP API handles responses directly
};

ControlInterface::ControlInterface() 
    : initialized(false), maxPWM(255), minPWM(0), safetyEnabled(true), 
      maxCurrent(10.0f), emergencyStopPWM(0) {
}

ControlInterface::~ControlInterface() {
    end();
}

void ControlInterface::begin() {
    if (initialized) return;
    
    const char* TAG = "ControlInterface::begin";
    
    // Initialize lights GPIO
    if (lightsPin != GPIO_NUM_NC) {
        gpio_config_t gpioConfig = {
            .pin_bit_mask = (1ULL << lightsPin),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        ESP_ERROR_CHECK(gpio_config(&gpioConfig));
        
        // Initialize lights to off state
        gpio_set_level(lightsPin, 0);
        lightsOn = false;
    }
    
    // Initialize PWM to safe state
    currentPWM = 0;
    
    initialized = true;
    ESP_LOGI(TAG, "Control interface initialized");
}

void ControlInterface::end() {
    if (initialized) {
        // Emergency stop - set all controls to safe state
        emergencyStop();
        initialized = false;
        ESP_LOGI("ControlInterface", "Control interface stopped");
    }
}

bool ControlInterface::processCommand(const String& command, const JsonDocument& data) {
    if (!initialized) {
        ESP_LOGE("ControlInterface", "Interface not initialized");
        return false;
    }
    
    if (!validateCommand(command)) {
        ESP_LOGW("ControlInterface", "Invalid command: %s", command.c_str());
        return false;
    }
    
    ESP_LOGI("ControlInterface", "Processing command: %s", command.c_str());
    
    bool success = false;
    String message = "";
    
    if (command == "setPWM") {
        if (data.containsKey("value")) {
            uint8_t pwm = data["value"].as<uint8_t>();
            success = setPWM(pwm);
            message = success ? "PWM set to " + String(pwm) : "Failed to set PWM";
        } else {
            message = "Missing PWM value";
        }
        
    } else if (command == "toggleDoors") {
        success = toggleDoors();
        message = success ? (doorsLocked ? "Doors locked" : "Doors unlocked") : "Failed to toggle doors";
        
    } else if (command == "toggleLights") {
        success = toggleLights();
        message = success ? (lightsOn ? "Lights on" : "Lights off") : "Failed to toggle lights";
        
    } else if (command == "lockDoors") {
        success = lockDoors();
        message = success ? "Doors locked" : "Failed to lock doors";
        
    } else if (command == "unlockDoors") {
        success = unlockDoors();
        message = success ? "Doors unlocked" : "Failed to unlock doors";
        
    } else if (command == "setLights") {
        if (data.containsKey("state")) {
            bool state = data["state"].as<bool>();
            success = setLights(state);
            message = success ? (state ? "Lights on" : "Lights off") : "Failed to set lights";
        } else {
            message = "Missing lights state";
        }
        
    } else if (command == "emergencyStop") {
        emergencyStop();
        success = true;
        message = "Emergency stop activated";
        
    } else if (command == "getStatus") {
        success = true;
        message = "Status available via HTTP API";
        
    } else {
        message = "Unknown command: " + command;
    }
    
    return success;
}

// WebSocket message processing removed - now handled by HTTP API

bool ControlInterface::setPWM(uint8_t pwm) {
    if (!validatePWMValue(pwm)) {
        ESP_LOGW("ControlInterface", "Invalid PWM value: %d", pwm);
        return false;
    }
    
    if (safetyEnabled && !checkSafetyLimits()) {
        ESP_LOGW("ControlInterface", "Safety limits exceeded, PWM command ignored");
        return false;
    }
    
    // Set PWM for both motors (synchronized)
    motorPWM1.setPwm(pwm);
    motorPWM2.setPwm(pwm);
    currentPWM = pwm;
    
    ESP_LOGI("ControlInterface", "PWM set to %d", pwm);
    return true;
}

bool ControlInterface::toggleDoors() {
    if (doorsLocked) {
        return unlockDoors();
    } else {
        return lockDoors();
    }
}

bool ControlInterface::toggleLights() {
    return setLights(!lightsOn);
}

bool ControlInterface::lockDoors() {
    doorLock.lockAllDoors();
    doorsLocked = true;
    ESP_LOGI("ControlInterface", "Doors locked");
    return true;
}

bool ControlInterface::unlockDoors() {
    doorLock.unlockAllDoors();
    doorsLocked = false;
    ESP_LOGI("ControlInterface", "Doors unlocked");
    return true;
}

bool ControlInterface::setLights(bool state) {
    controlLights(state);
    lightsOn = state;
    ESP_LOGI("ControlInterface", "Lights %s", state ? "on" : "off");
    return true;
}

void ControlInterface::controlLights(bool state) {
    if (lightsPin != GPIO_NUM_NC) {
        gpio_set_level(lightsPin, state ? 1 : 0);
    }
}

void ControlInterface::controlDoors(bool lock) {
    if (lock) {
        doorLock.lockAllDoors();
    } else {
        doorLock.unlockAllDoors();
    }
    doorsLocked = lock;
}

void ControlInterface::controlMotorPWM(uint8_t pwm) {
    motorPWM1.setPwm(pwm);
    motorPWM2.setPwm(pwm);
    currentPWM = pwm;
}

bool ControlInterface::validatePWMValue(uint8_t pwm) {
    return (pwm >= minPWM && pwm <= maxPWM);
}

bool ControlInterface::validateCommand(const String& command) {
    const String validCommands[] = {
        "setPWM", "toggleDoors", "toggleLights", "lockDoors", "unlockDoors", 
        "setLights", "emergencyStop", "getStatus"
    };
    
    for (const String& validCmd : validCommands) {
        if (command == validCmd) {
            return true;
        }
    }
    return false;
}

bool ControlInterface::checkSafetyLimits() {
    // Check current limits (would need access to current sensor data)
    // For now, always return true - implement actual safety checks
    return true;
}

void ControlInterface::emergencyStop() {
    ESP_LOGW("ControlInterface", "EMERGENCY STOP ACTIVATED");
    
    // Stop motors immediately
    controlMotorPWM(emergencyStopPWM);
    
    // Turn off lights
    controlLights(false);
    
    // Lock doors for safety
    controlDoors(true);
    
    // Emergency stop status is now logged only - HTTP API provides status updates
}

void ControlInterface::setSafetyLimits(float maxCurrentAmps, uint8_t maxPwmValue) {
    maxCurrent = maxCurrentAmps;
    maxPWM = (maxPwmValue <= 255) ? maxPwmValue : 255;
    ESP_LOGI("ControlInterface", "Safety limits set: Max current=%.1fA, Max PWM=%d", maxCurrent, maxPWM);
}

void ControlInterface::enableSafety(bool enable) {
    safetyEnabled = enable;
    ESP_LOGI("ControlInterface", "Safety %s", enable ? "enabled" : "disabled");
}

// sendResponse method removed - HTTP API handles responses directly

// sendStatus method removed - HTTP API provides status via /api/motor/status endpoint