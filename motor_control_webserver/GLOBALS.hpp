#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <atomic>
#include <Preferences.h>

#include "PwmGenerator.hpp"
#include "BLDCPulseCalculator.hpp"
#include "MotorDirection.hpp"
#include "SingleDoorLock.hpp"
#include "Light.hpp"
#include "UARTCurrentSensor.hpp"
#include "WebServerManager.hpp"
#include "DataCollector.hpp"

// WiFi Configuration - Station/Client Mode (connect to existing network)
const char* WIFI_SSID = "GRIDFLOW_2.4G";
const char* WIFI_PASSWORD = "Gridflow@2024";

// WiFi Access Point Configuration (ESP32 becomes hotspot)
const char* AP_SSID = "BLDC_Motor_Control";
const char* AP_PASSWORD = "BlDcMoToR2024";  // Must be at least 8 characters
const IPAddress AP_IP(192, 168, 4, 1);        // Default AP IP
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);

// WiFi Mode Selection - Now loaded from NVS (Non-Volatile Storage)
// Default fallback values if NVS is empty
const bool DEFAULT_USE_AP_MODE = true;  // Default to field mode for safety and reliability

// Global variables for runtime mode switching
extern bool currentUseAPMode;
extern Preferences preferences;

// FreeRTOS Core Assignment
static const BaseType_t app_cpu0 = 0;  // Motor control, current sensing, hardware operations
static const BaseType_t app_cpu1 = 1;  // Web server, HTTP API, speed calculation, data processing

// Motor Properties
const uint32_t frequency = 100000;                            // PWM frequency in Hz
const ledc_timer_bit_t resolution = LEDC_TIMER_8_BIT;         // PWM resolution (8-bit)

// ESP32 DevKit V1 Pin Configuration - UPDATED TO MATCH ACTUAL HARDWARE
// Motor 1 Configuration (matches BLDC_speed_Arduino hardware)
constexpr uint8_t motorId1 = 1;
constexpr gpio_num_t feedBackPin1 = GPIO_NUM_2;      // Feedback pulse input (actual hardware)
constexpr gpio_num_t motorPwmPin1 = GPIO_NUM_12;     // PWM capable pin (actual hardware)
constexpr gpio_num_t triggerPin1 = GPIO_NUM_19;      // Motor direction control (actual hardware)

// Motor 2 Configuration (matches BLDC_speed_Arduino hardware)
constexpr uint8_t motorId2 = 2;
constexpr gpio_num_t feedBackPin2 = GPIO_NUM_15;     // Feedback pulse input (GPIO 35 - input only, no boot conflicts)
constexpr gpio_num_t motorPwmPin2 = GPIO_NUM_14;     // PWM capable pin (actual hardware)
constexpr gpio_num_t triggerPin2 = GPIO_NUM_23;      // Motor direction control (actual hardware)

// LED Lights Control (via IRF520N MOSFET) - Updated for 2 lights
constexpr gpio_num_t leftLightPin = GPIO_NUM_17;    // Left LED lights control via IRF520N MOSFET
constexpr gpio_num_t rightLightPin = GPIO_NUM_16;    // Right LED lights control via IRF520N MOSFET

// 4-Door Solenoid Control (via IRF520N MOSFETs) - REASSIGNED TO AVOID MOTOR PIN CONFLICTS
constexpr gpio_num_t doorLock1Pin = GPIO_NUM_25;     // Front Left Door via IRF520N
constexpr gpio_num_t doorLock2Pin = GPIO_NUM_26;     // Front Right Door via IRF520N  
constexpr gpio_num_t doorLock3Pin = GPIO_NUM_32;     // Rear Left Door via IRF520N
constexpr gpio_num_t doorLock4Pin = GPIO_NUM_33;     // Rear Right Door via IRF520N

// UART Current Sensor Communication (Arduino Nano via Hardware UART2)
// ESP32 DevKit V1 UART2 pins: TX=GPIO17, RX=GPIO16 (Hardware UART)
// Arduino Nano Hardware Serial pins: TX=D1, RX=D0
constexpr gpio_num_t UART_TX_PIN = GPIO_NUM_4;     // UART2 TX to Arduino Nano RX
constexpr gpio_num_t UART_RX_PIN = GPIO_NUM_5;     // UART2 RX from Arduino Nano TX

// Physical Emergency WiFi Mode Switch Button
constexpr gpio_num_t WIFI_MODE_BUTTON_PIN = GPIO_NUM_0;  // Boot button (GPIO 0) - built-in on ESP32 DevKit

// Global Objects
extern PwmGenerator motorPWM1;
extern PwmGenerator motorPWM2;
extern BLDCPulseCalculator motorPulse1;
extern BLDCPulseCalculator motorPulse2;
extern MotorDirection direction;        // Future implementation

// New modular objects
extern Light leftLight;               // Left LED light control
extern Light rightLight;             // Right LED light control
extern SingleDoorLock frontLeftDoor; // Front Left Door lock
extern SingleDoorLock frontRightDoor;// Front Right Door lock
extern SingleDoorLock rearLeftDoor;  // Rear Left Door lock
extern SingleDoorLock rearRightDoor; // Rear Right Door lock
extern UARTCurrentSensor currentSensor; // UART communication with Arduino Nano

extern WebServerManager webServer;
extern DataCollector dataCollector;

// Task Handles
extern TaskHandle_t motor1SpeedTaskHandle;
extern TaskHandle_t motor2SpeedTaskHandle;
extern TaskHandle_t motor1PWMTaskHandle;
extern TaskHandle_t motor2PWMTaskHandle;
extern TaskHandle_t currentSensorTaskHandle;
extern TaskHandle_t dataCollectorTaskHandle;

// Global State Variables  
extern bool doorsLocked;           // All doors lock state (legacy)
extern bool door1Locked;           // Front Left Door lock state
extern bool door2Locked;           // Front Right Door lock state  
extern bool door3Locked;           // Rear Left Door lock state
extern bool door4Locked;           // Rear Right Door lock state
extern bool leftLightOn;           // Left light state
extern bool rightLightOn;          // Right light state
extern uint8_t currentPWM;         // Current PWM value (0-255)
extern float motor1Speed;          // Motor 1 speed in RPM
extern float motor2Speed;          // Motor 2 speed in RPM
extern float motor1Current;        // Motor 1 current in Amps
extern float motor2Current;        // Motor 2 current in Amps
extern float systemVoltage;        // System voltage

// Global Variable Definitions (to avoid linker errors)
bool doorsLocked = false;
bool door1Locked = false;
bool door2Locked = false;
bool door3Locked = false;
bool door4Locked = false;
bool leftLightOn = false;
bool rightLightOn = false;
uint8_t currentPWM = 0;
float motor1Speed = 0.0f;
float motor2Speed = 0.0f;
float motor1Current = 0.0f;
float motor2Current = 0.0f;
float systemVoltage = 0.0f;
