#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <atomic>

#include "PwmGenerator.hpp"
#include "BLDCPulseCalculator.hpp"
#include "MotorDirection.hpp"
#include "DoorLock.hpp"
#include "UARTCurrentSensor.hpp"
#include "WebServerManager.hpp"
// WebSocket removed - using HTTP polling instead
#include "DataCollector.hpp"
#include "ControlInterface.hpp"

// WiFi Configuration
const char* WIFI_SSID = "GRIDFLOW_2.4G";
const char* WIFI_PASSWORD = "Gridflow@2024";

// FreeRTOS Core Assignment
static const BaseType_t app_cpu0 = 0;  // Motor control, current sensing
static const BaseType_t app_cpu1 = 1;  // Web server, WebSocket, speed calculation

// Motor Properties
const uint32_t frequency = 100000;                            // PWM frequency in Hz
const ledc_timer_bit_t resolution = LEDC_TIMER_8_BIT;         // PWM resolution (8-bit)

// ESP32 DevKit V1 Pin Configuration
// Motor 1 Configuration  
constexpr uint8_t motorId1 = 1;
constexpr gpio_num_t feedBackPin1 = GPIO_NUM_32;     // ADC1_CH4 - Feedback pulse input
constexpr gpio_num_t motorPwmPin1 = GPIO_NUM_25;     // PWM capable pin
constexpr gpio_num_t triggerPin1 = GPIO_NUM_33;      // ADC1_CH5 - Future motor direction

// Motor 2 Configuration  
constexpr uint8_t motorId2 = 2;
constexpr gpio_num_t feedBackPin2 = GPIO_NUM_35;     // ADC1_CH7 - Feedback pulse input
constexpr gpio_num_t motorPwmPin2 = GPIO_NUM_26;     // PWM capable pin
constexpr gpio_num_t triggerPin2 = GPIO_NUM_34;      // ADC1_CH6 - Future motor direction

// LED Lights Control (via IRF520N MOSFET)
extern gpio_num_t lightsPin;        // LED lights control via IRF520N MOSFET

// 4-Door Solenoid Control (via IRF520N MOSFETs)
constexpr gpio_num_t doorLock1Pin = GPIO_NUM_12;     // Front Left Door via IRF520N
constexpr gpio_num_t doorLock2Pin = GPIO_NUM_13;     // Front Right Door via IRF520N
constexpr gpio_num_t doorLock3Pin = GPIO_NUM_14;     // Rear Left Door via IRF520N
constexpr gpio_num_t doorLock4Pin = GPIO_NUM_15;     // Rear Right Door via IRF520N

// UART Current Sensor Communication (Arduino Nano via Hardware UART2)
// ESP32 DevKit V1 UART2 pins: TX=GPIO17, RX=GPIO16 (Hardware UART)
// Arduino Nano Hardware Serial pins: TX=D1, RX=D0
constexpr gpio_num_t UART_TX_PIN = GPIO_NUM_17;     // UART2 TX to Arduino Nano RX
constexpr gpio_num_t UART_RX_PIN = GPIO_NUM_16;     // UART2 RX from Arduino Nano TX

// Global Objects
extern PwmGenerator motorPWM1;
extern PwmGenerator motorPWM2;
extern BLDCPulseCalculator motorPulse1;
extern BLDCPulseCalculator motorPulse2;
extern MotorDirection direction;        // Future implementation
extern DoorLock doorLock;              // Door solenoid control
extern UARTCurrentSensor currentSensor; // UART communication with Arduino Nano

extern WebServerManager webServer;
// WebSocket manager removed - using HTTP polling instead
extern DataCollector dataCollector;
extern ControlInterface controlInterface;

// Task Handles
extern TaskHandle_t motor1SpeedTaskHandle;
extern TaskHandle_t motor2SpeedTaskHandle;
extern TaskHandle_t motor1PWMTaskHandle;
extern TaskHandle_t motor2PWMTaskHandle;
extern TaskHandle_t currentSensorTaskHandle;
extern TaskHandle_t dataCollectorTaskHandle;
extern TaskHandle_t webSocketTaskHandle;

// Global State Variables  
extern bool doorsLocked;           // All doors lock state
extern bool door1Locked;           // Front Left Door lock state
extern bool door2Locked;           // Front Right Door lock state  
extern bool door3Locked;           // Rear Left Door lock state
extern bool door4Locked;           // Rear Right Door lock state
extern bool lightsOn;              // Lights state
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
bool lightsOn = false;
uint8_t currentPWM = 0;
float motor1Speed = 0.0f;
float motor2Speed = 0.0f;
float motor1Current = 0.0f;
float motor2Current = 0.0f;
float systemVoltage = 0.0f;
gpio_num_t lightsPin = GPIO_NUM_27;