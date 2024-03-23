#include <stdio.h>
#include "uart.hpp"
#include "BLDCPulseCalculator.hpp"
#include "oledFunctions.hpp"
#include "pwmGenerator.hpp"
#include "MotorDirection.hpp"
#include <U8g2lib.h>



// Use only core 1 for demo purposes
// #if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu0 = 0;
// #else
  static const BaseType_t app_cpu1 = 1;
// #endif

// Motor1 
constexpr gpio_num_t wavePin1 = GPIO_NUM_2;
constexpr gpio_num_t wavePin2 = GPIO_NUM_15;

//PwmGenerator
constexpr gpio_num_t motorPwm1 = GPIO_NUM_12;
constexpr gpio_num_t motorPwm2 = GPIO_NUM_14; 

const uint32_t frequency = 100000;           // PWM frequency in Hz
const uint8_t resolution = 8;        // PWM resolution (8-bit)

constexpr uint8_t motorId1 = 1;
constexpr uint8_t motorId2 = 2;

//Relay module
constexpr gpio_num_t relayPin1 = GPIO_NUM_19;
constexpr gpio_num_t relayPin2 = GPIO_NUM_23;

// Uart
constexpr gpio_num_t RXD = GPIO_NUM_5;
constexpr gpio_num_t TXD = GPIO_NUM_4;
constexpr uint8_t RTS = (UART_PIN_NO_CHANGE);
constexpr uint8_t CTS = (UART_PIN_NO_CHANGE);
constexpr uart_port_t PORT_NUM = UART_NUM_2;
constexpr int UART_BAUD_RATE = 115200;


/* Object creation */

// BLDCPulseCalculator
BLDCPulseCalculator PMSMMotor1(wavePin1, motorId1);
BLDCPulseCalculator PMSMMotor2(wavePin2, motorId2);

// PwmGenerator
PwmGenerator motor1(motorPwm1, frequency, resolution);
PwmGenerator motor2(motorPwm2, frequency, resolution);

// Uart object
HardwareUart serial(RXD, TXD, RTS, CTS, PORT_NUM, UART_BAUD_RATE);

// Motor Direction
MotorDirection direction(relayPin1, relayPin2);

char data[40];
