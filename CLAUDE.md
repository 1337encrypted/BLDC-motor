# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This repository contains multiple BLDC (Brushless DC) motor control projects for ESP32 microcontrollers. The codebase focuses on motor synchronization, speed measurement, current sensing, and web-based control interfaces for PMSM (Permanent Magnet Synchronous Motor) applications.

## Architecture

The project is organized into several distinct modules:

### Core Components

1. **BLDC_speed/** - ESP-IDF based motor control system
   - Uses ESP-IDF framework with CMake build system
   - Implements FreeRTOS tasks for motor pulse calculation
   - Includes UART communication and OLED display functionality

2. **BLDC_speed_Arduino/** - Arduino IDE compatible version  
   - Arduino framework implementation of motor control
   - Supports dual motor operation with Bluetooth communication
   - Integrates PWM generation, direction control, and data logging

3. **Current_Sensor/** - Current monitoring system
   - Uses ACS712 current sensors and PC817A optocouplers
   - Implements overcurrent protection with buzzer alerts
   - Arduino-based with serial communication interface

4. **motor_control_webserver/** - Modern web-based motor control system
   - ESP32-based web server with HTTP REST API (replaced WebSocket for stability)
   - Complete web dashboard with HTML/CSS/JavaScript interface
   - Dual motor control with synchronized PWM generation
   - Current sensing via UART communication with Arduino Nano
   - 4-door solenoid control system and LED lighting control
   - Emergency stop functionality and safety monitoring
   - File upload system for web interface management

### Key Classes and Modules

#### Motor Control Classes
- **BLDCPulseCalculator** - Core motor speed measurement using pulse counting with interrupt-driven timing
- **PwmGenerator** - PWM signal generation with FreeRTOS task management (100kHz, 8-bit resolution)
- **MotorDirection** - Direction control logic for synchronized motor operation
- **HardwareUart** - Serial communication handling for data transmission
- **dataLogger** - Bluetooth data transmission for real-time monitoring

#### Web Server Classes (motor_control_webserver/)
- **WebServerManager** - ESP32 built-in WebServer with HTTP REST API endpoints, static file serving, and file upload functionality
- **DataCollector** - Aggregates motor data, current readings, and system status for API transmission
- **ControlInterface** - Processes HTTP API commands and converts them to motor control actions (WebSocket removed for stability)
- **DoorLock** - 4-door solenoid control system with individual and group control methods
- **UARTCurrentSensor** - Communication with Arduino Nano for ACS712 current sensor readings

#### Hardware Integration Classes
- **handleMotorInput** - Motor synchronization and input processing for dual motor coordination

## Build Systems

### ESP-IDF Projects (BLDC_speed/)
```bash
# Build ESP-IDF project
idf.py build

# Flash to device  
idf.py flash

# Monitor serial output
idf.py monitor

# Clean build
idf.py clean
```

### Arduino Projects

#### Traditional Arduino Projects (BLDC_speed_Arduino/, Current_Sensor/)
- Use Arduino IDE or PlatformIO
- Select ESP32 board configuration
- Required libraries: BluetoothSerial, U8g2 (for OLED), ESP32 driver libraries

#### Web Server Project (motor_control_webserver/)
**Compatible Library Versions (Critical for Compilation):**
```bash
# Required libraries with exact versions for ESP32 Core 3.3.0:
- ArduinoJson: v6.21.5 (NOT v7.x)
- ESP32 Core: v3.3.0
- LittleFS: Built-in with ESP32 core
- WebServer: Built-in with ESP32 core (replaced AsyncWebServer for stability)
```

**IMPORTANT: AsyncWebServer and AsyncTCP libraries are NO LONGER USED**
- Replaced with built-in ESP32 WebServer library to resolve TCPIP core locking issues
- WebSocket functionality removed - using HTTP REST API polling instead

**Board Configuration:**
```
Board: ESP32 Dev Module
Upload Speed: 921600
CPU Frequency: 240MHz (WiFi/BT)
Flash Frequency: 80MHz
Flash Mode: DIO
Flash Size: 4MB (32Mb)
Partition Scheme: Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)
```

**Compilation Notes:**
- Uses ArduinoJson v6 syntax with DynamicJsonDocument
- HTTP REST API endpoints replace WebSocket communication for stability
- Built-in WebServer library eliminates AsyncWebServer compatibility issues
- FreeRTOS task management with dual-core CPU utilization
- TCPIP core locking issues resolved by removing AsyncWebServer

## Hardware Configuration

### Motor Setup (ESP32 DevKit V1)
#### BLDC_speed_Arduino Configuration:
- **Motor 1**: Feedback pin GPIO_2, PWM pin GPIO_12, Trigger pin GPIO_19
- **Motor 2**: Feedback pin GPIO_15, PWM pin GPIO_14, Trigger pin GPIO_23

#### motor_control_webserver Configuration:
- **Motor 1**: PWM pin GPIO_25, Feedback pin GPIO_32, Trigger pin GPIO_33
- **Motor 2**: PWM pin GPIO_26, Feedback pin GPIO_35, Trigger pin GPIO_34
- **LED Lights**: GPIO_27 (via IRF520N MOSFET)
- **4-Door Control**: GPIO_12,13,14,15 (via IRF520N MOSFETs)
- **UART Current Sensing**: TX=GPIO_17, RX=GPIO_16 (to Arduino Nano)

### PWM Configuration
- **PWM Frequency**: 100kHz with 8-bit resolution (0-255 duty cycle)
- **Current Sensors**: ACS712 with 8.0A overcurrent threshold
- **Safety Features**: Emergency stop, current monitoring, PWM limits

### Communication
#### Traditional Projects:
- **UART**: 115200 baud rate for serial communication
- **Bluetooth**: "Pod1_Bluetooth" device name for wireless data transmission

#### Web Server Project:
- **WiFi**: 2.4GHz network connection for web interface access
- **HTTP REST API**: Synchronous communication via HTTP endpoints (replaced WebSocket)
- **UART**: 115200 baud communication with Arduino Nano current sensor
- **Core Assignment**: Tasks distributed across ESP32 dual cores (app_cpu0, app_cpu1)
  - Core 0: Motor PWM generation, current sensing, hardware control
  - Core 1: Web server, HTTP API, speed calculation, data processing

## Development Patterns

### Task Management
The codebase extensively uses FreeRTOS tasks with specific core assignments:

#### Traditional Projects:
- Core 0: Left motor operations, data logging
- Core 1: Right motor operations, display rendering, input handling

#### Web Server Project:
- Core 0: Motor PWM generation, current sensing, door control, hardware operations
- Core 1: Web server, HTTP API communication, speed calculation, data collection, processing

### Global Configuration
Each module has a GLOBALS.hpp file containing:
- Pin definitions and hardware configuration
- Object instantiation for all major components  
- Task handle declarations
- Core assignment constants
- Global state variables with proper extern declarations

### Modular Design
Components are designed as self-contained classes with:
- `.begin()` methods for initialization
- Task creation with specific core assignment
- Hardware abstraction for different pin configurations
- Modern C++ practices with proper memory management
- WebSocket message handling with structured JSON communication

### Code Quality Standards
#### Web Server Project Improvements:
- **Memory Safety**: Buffer overflow protection in WebSocket message handling
- **Library Compatibility**: Updated to use ArduinoJson v6 syntax and AsyncResponseStream
- **Error Handling**: Comprehensive error checking and logging throughout
- **Resource Management**: Proper cleanup and task lifecycle management
- **Type Safety**: Correct constructor signatures and parameter matching

## Current Project Status

### Completed Components
✅ **motor_control_webserver/** - Fully implemented and debugged
- All compilation errors resolved (AsyncJsonResponse, ArduinoJson v6, buffer safety)
- Memory management optimized with proper DynamicJsonDocument usage
- Library compatibility issues addressed with specific version requirements
- Web interface (HTML/CSS/JavaScript) complete with modern responsive design
- Real-time WebSocket communication implemented with 10Hz update rate
- Motor control, door control, and lighting systems fully integrated
- Emergency stop and safety monitoring systems operational

✅ **Traditional Arduino Projects** - Stable and functional
- BLDC_speed_Arduino/ - Bluetooth-based motor control with dual motor support
- Current_Sensor/ - ACS712 current monitoring with OLED display and buzzer alerts
- BLDC_speed/ - ESP-IDF based implementation with CMake build system

### Ready for Deployment
The motor_control_webserver project is production-ready with:
- Comprehensive error handling and safety features
- Real-time web dashboard accessible from any browser on the network
- WebSocket communication at 10Hz update rate for smooth real-time control
- Emergency stop and overcurrent protection with automatic motor shutdown
- Dual-core FreeRTOS task management for optimal performance and responsiveness
- Complete hardware abstraction for easy pin configuration changes

### Web Interface Features
- **Real-time Motor Control**: PWM slider with preset buttons (0%, 25%, 50%, 75%, 100%)
- **Live Data Display**: Motor speeds (RPM), current consumption (A), system voltage (V)
- **Hardware Control**: 4-door lock/unlock system, LED lighting control
- **System Monitoring**: WiFi signal strength, system uptime, memory usage
- **Safety Features**: Emergency stop button, connection status monitoring
- **Mobile Responsive**: Touch-friendly interface that works on phones and tablets

### Next Steps for Deployment
1. **Install Compatible Libraries**: Use exact versions specified in CLAUDE.md
2. **Upload Web Files**: Use ESP32 LittleFS Data Upload tool for data/ folder
3. **Configure WiFi**: Edit WIFI_SSID and WIFI_PASSWORD in GLOBALS.hpp
4. **Upload Arduino Sketch**: Standard upload process to ESP32
5. **Access Web Interface**: Navigate to ESP32 IP address in any web browser
6. **Optional Current Monitoring**: Upload current_sensor_nano.ino to Arduino Nano for real-time current sensing