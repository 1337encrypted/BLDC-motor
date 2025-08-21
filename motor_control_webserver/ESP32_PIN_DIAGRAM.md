# ESP32 DevKit V1 Pin Configuration - BLDC Motor Control System

## Hardware Overview
This system uses ESP32 DevKit V1 for dual-core motor control, web interface, and hardware management.

## Pin Mapping Table

| GPIO Pin | Function | Hardware Connection | Direction | Description |
|----------|----------|---------------------|-----------|-------------|
| **GPIO 2** | Motor 1 Feedback | Pulse Encoder Output | Input | Motor 1 speed measurement (ACTUAL HARDWARE) |
| **GPIO 4** | Right Light Control | IRF520N MOSFET Gate | Output | Right LED lights via MOSFET |
| **GPIO 12** | Motor 1 PWM | Motor Driver PWM Input | Output | Motor 1 speed control (100kHz, 8-bit) |
| **GPIO 14** | Motor 2 PWM | Motor Driver PWM Input | Output | Motor 2 speed control (100kHz, 8-bit) |
| **GPIO 15** | Motor 2 Feedback | Pulse Encoder Output | Input | Motor 2 speed measurement (ACTUAL HARDWARE) |
| **GPIO 16** | UART RX | Arduino Nano TX | Input | Current sensor data receive |
| **GPIO 17** | UART TX | Arduino Nano RX | Output | Current sensor communication |
| **GPIO 19** | Motor 1 Direction | Motor Driver DIR | Output | Motor 1 direction control (ACTUAL HARDWARE) |
| **GPIO 23** | Motor 2 Direction | Motor Driver DIR | Output | Motor 2 direction control (ACTUAL HARDWARE) |
| **GPIO 25** | Door Lock 1 | IRF520N MOSFET Gate | Output | Front Left Door solenoid |
| **GPIO 26** | Door Lock 2 | IRF520N MOSFET Gate | Output | Front Right Door solenoid |
| **GPIO 27** | Left Light Control | IRF520N MOSFET Gate | Output | Left LED lights via MOSFET |
| **GPIO 32** | Door Lock 3 | IRF520N MOSFET Gate | Output | Rear Left Door solenoid |
| **GPIO 33** | Door Lock 4 | IRF520N MOSFET Gate | Output | Rear Right Door solenoid |

## Power and Communication Pins

| Pin | Function | Connection |
|-----|----------|------------|
| **3V3** | Power Supply | 3.3V output for sensors |
| **5V** | Power Supply | 5V input/output |
| **GND** | Ground | Common ground |
| **EN** | Enable | Reset/Enable pin |

## Detailed Hardware Connections

### Motor Control System (UPDATED TO MATCH ACTUAL HARDWARE)
```
Motor 1:
├── PWM Control: GPIO 12 → Motor Driver PWM Input
├── Speed Feedback: Motor Encoder → GPIO 2 (ACTUAL HARDWARE)
└── Direction Control: GPIO 19 → Motor Driver DIR (ACTUAL HARDWARE)

Motor 2:
├── PWM Control: GPIO 14 → Motor Driver PWM Input  
├── Speed Feedback: Motor Encoder → GPIO 15 (ACTUAL HARDWARE)
└── Direction Control: GPIO 23 → Motor Driver DIR (ACTUAL HARDWARE)
```

### LED Lighting System (via IRF520N MOSFETs)
```
Left Lights:
GPIO 27 → IRF520N Gate → LED Strip/Lights → 12V Supply

Right Lights:
GPIO 4 → IRF520N Gate → LED Strip/Lights → 12V Supply

IRF520N Wiring:
├── Gate: ESP32 GPIO
├── Drain: LED Strip Positive
├── Source: GND
└── Load: LED Strip → 12V Supply
```

### 4-Door Lock System (via IRF520N MOSFETs) - UPDATED PINS
```
Front Left Door:
GPIO 25 → IRF520N Gate → Solenoid Lock → 12V Supply

Front Right Door:
GPIO 26 → IRF520N Gate → Solenoid Lock → 12V Supply

Rear Left Door:
GPIO 32 → IRF520N Gate → Solenoid Lock → 12V Supply

Rear Right Door:
GPIO 33 → IRF520N Gate → Solenoid Lock → 12V Supply

Solenoid Wiring (each):
├── Gate: ESP32 GPIO
├── Drain: Solenoid Positive
├── Source: GND
└── Load: Solenoid → 12V Supply
```

### Current Sensor Communication (UART2)
```
ESP32 Hardware UART2:
├── TX (GPIO 17) → Arduino Nano RX (D0)
└── RX (GPIO 16) ← Arduino Nano TX (D1)

Arduino Nano Current Sensors:
├── ACS712 Sensor 1 → A0 (Motor 1 Current)
├── ACS712 Sensor 2 → A1 (Motor 2 Current)
├── Voltage Divider → A2 (System Voltage)
└── PC817A Optocoupler → D2 (Overcurrent Alert)
```

## Core Assignment Strategy

### Core 0 (Hardware Control)
- Motor PWM Generation (PwmGenerator tasks)
- Current Sensor Communication (UART tasks)
- Door Lock Control (GPIO operations)
- LED Light Control (GPIO operations)
- Hardware interrupts and timing-critical operations

### Core 1 (Processing & Communication)
- Web Server (HTTP requests)
- Data Collection (DataCollector task)
- Speed Calculation (BLDCPulseCalculator tasks)
- WiFi Communication
- JSON processing and API responses

## Pin States and Logic Levels

### Output Pins (Active HIGH)
| Pin | LOW State | HIGH State |
|-----|-----------|------------|
| GPIO 12/14 | Motor OFF (0% PWM) | Motor ON (PWM duty cycle) |
| GPIO 19/23 | Direction Control | Direction Control |
| GPIO 27/4 | Lights OFF | Lights ON |
| GPIO 25/26/32/33 | Doors UNLOCKED | Doors LOCKED |
| GPIO 17 | UART Idle | UART Data |

### Input Pins
| Pin | Signal Type | Frequency Range |
|-----|-------------|-----------------|
| GPIO 2/15 | Pulse Train | 0-10kHz (motor feedback) |
| GPIO 16 | UART Serial | 115200 baud |

## Safety Features

### Hardware Protection
- IRF520N MOSFETs provide isolation between ESP32 and high-current loads
- Current sensors monitor motor current for overcurrent protection
- UART communication provides real-time current monitoring
- Emergency stop functionality via software and potential hardware button

### Software Protection
- PWM limits (0-255, 8-bit resolution)
- Current monitoring with automatic shutdown
- Dual-core architecture prevents blocking operations
- FreeRTOS task management with priority scheduling

## Power Requirements

### ESP32 DevKit V1
- Operating Voltage: 3.3V (regulated from 5V USB/VIN)
- Current Consumption: ~240mA (both cores active, WiFi on)

### External Components
- Motor Drivers: 12V supply (current depends on motors)
- LED Lights: 12V supply via IRF520N MOSFETs
- Door Lock Solenoids: 12V supply via IRF520N MOSFETs
- Arduino Nano: 5V supply (for current sensors)

## Communication Protocols

### WiFi (2.4GHz)
- HTTP Web Server on port 80
- REST API endpoints for motor control
- Real-time status updates via HTTP polling

### UART (Hardware Serial2)
- Baud Rate: 115200 bps
- Data Format: 8N1 (8 data bits, no parity, 1 stop bit)
- Communication with Arduino Nano for current sensing

### PWM (LEDC)
- Frequency: 100kHz
- Resolution: 8-bit (0-255)
- Timer: LEDC peripheral for hardware PWM generation

## Development Notes

### Pin Change History
- **MAJOR UPDATE**: Motor pins changed to match actual hardware (BLDC_speed_Arduino)
- **GPIO 2**: Now correctly assigned to Motor 1 feedback (actual hardware)
- **GPIO 12**: Now correctly assigned to Motor 1 PWM (actual hardware)  
- **GPIO 14**: Now correctly assigned to Motor 2 PWM (actual hardware)
- **GPIO 15**: Now correctly assigned to Motor 2 feedback (actual hardware)
- **GPIO 19**: Now correctly assigned to Motor 1 direction (actual hardware)
- **GPIO 23**: Now correctly assigned to Motor 2 direction (actual hardware)
- **Door Locks**: Reassigned to GPIO 25,26,32,33 to avoid motor pin conflicts

### Future Expansion
- Available pins: GPIO 18, GPIO 21, GPIO 22, GPIO 34, GPIO 35
- Additional sensors or controls can be added to available pins
- Current pin assignment matches proven working hardware

### Debugging Access
- Serial Monitor: USB connection for debugging output
- GPIO 2: Built-in LED for status indication
- UART pins accessible for logic analyzer connection

This pin configuration provides a robust, expandable motor control system with comprehensive monitoring and safety features.