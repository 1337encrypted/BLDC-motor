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

4. **motor_control_webserver/** - Modern web-based motor control system ✅ **PRODUCTION READY**
   - ESP32-based web server with HTTP REST API (WebSocket removed for stability)
   - Complete responsive web dashboard with real-time updates via HTTP polling
   - Dual motor control with hardware-accurate pin configuration
   - Modular hardware control: 2 independent lights + 4 individual door locks
   - Current sensing via UART communication with Arduino Nano
   - Emergency stop functionality and comprehensive safety monitoring
   - Clean, refactored codebase with no legacy code

### Key Classes and Modules

#### Motor Control Classes
- **BLDCPulseCalculator** - Core motor speed measurement using pulse counting with interrupt-driven timing
- **PwmGenerator** - PWM signal generation with FreeRTOS task management (100kHz, 8-bit resolution)
- **MotorDirection** - Direction control logic for synchronized motor operation
- **HardwareUart** - Serial communication handling for data transmission
- **dataLogger** - Bluetooth data transmission for real-time monitoring

#### Web Server Classes (motor_control_webserver/) - **REFACTORED & OPTIMIZED**
- **WebServerManager** - ESP32 built-in WebServer with HTTP REST API endpoints and static file serving
- **DataCollector** - Real-time aggregation of motor data, current readings, and system status with 1-second polling
- **Light** - Modular individual light control class (2 instances: leftLight, rightLight)
- **SingleDoorLock** - Modular individual door lock control class (4 instances: FL, FR, RL, RR)
- **UARTCurrentSensor** - Arduino Nano communication for ACS712 current sensor readings

#### Removed Legacy Components (Cleanup Completed)
- ~~**ControlInterface**~~ - Removed (redundant with direct object control)
- ~~**DoorLock**~~ - Replaced with modular SingleDoorLock instances
- ~~**WebSocketManager**~~ - Removed (HTTP polling more stable than WebSocket)

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

#### Web Server Project (motor_control_webserver/) - **PRODUCTION READY**
**Compatible Library Versions (Verified Working):**
```bash
# Required libraries with exact versions for ESP32 Core 3.3.0:
- ArduinoJson: v6.21.5 (NOT v7.x) ✅ VERIFIED
- ESP32 Core: v3.3.0 ✅ VERIFIED 
- LittleFS: Built-in with ESP32 core ✅ VERIFIED
- WebServer: Built-in with ESP32 core ✅ VERIFIED
```

**Architecture Improvements Completed:**
- ✅ **HTTP Polling** replaces WebSocket for better stability and reliability
- ✅ **Modular Design** with individual Light and SingleDoorLock objects
- ✅ **Legacy Code Removed** - clean, maintainable codebase
- ✅ **Hardware-Accurate Pins** matching BLDC_speed_Arduino proven hardware
- ✅ **Real-time Synchronization** across multiple devices (1-second polling)

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

**LittleFS Data Upload:**
1. Install ESP32LittleFS Data Upload plugin in Arduino IDE
2. Place web files (index.html, script.js, style.css) in `data/` folder
3. Use `Tools > ESP32 LittleFS Data Upload` to upload web files
4. Upload must be done before uploading sketch for first time setup

**Compilation Notes:**
- Uses ArduinoJson v6 syntax with DynamicJsonDocument
- HTTP REST API endpoints replace WebSocket communication for stability
- Built-in WebServer library eliminates AsyncWebServer compatibility issues
- FreeRTOS task management with dual-core CPU utilization
- TCPIP core locking issues resolved by removing AsyncWebServer

## Hardware Configuration

### Hardware Configuration (ESP32 DevKit V1) - **UPDATED TO MATCH ACTUAL HARDWARE**

#### Motor Control Configuration (All Projects Now Consistent):
**Motor Pin Assignments (matches BLDC_speed_Arduino proven hardware):**
- **Motor 1**: PWM=GPIO_12, Feedback=GPIO_2, Direction=GPIO_19
- **Motor 2**: PWM=GPIO_14, Feedback=GPIO_15, Direction=GPIO_23

#### motor_control_webserver Additional Hardware:
**Modular Light Control (2 Independent Lights):**
- **Left Light**: GPIO_27 (via IRF520N MOSFET)
- **Right Light**: GPIO_4 (via IRF520N MOSFET)

**Modular Door Lock Control (4 Individual Locks):**
- **Front Left Door**: GPIO_25 (via IRF520N MOSFET)
- **Front Right Door**: GPIO_26 (via IRF520N MOSFET)
- **Rear Left Door**: GPIO_32 (via IRF520N MOSFET)
- **Rear Right Door**: GPIO_33 (via IRF520N MOSFET)

**Communication & Sensing:**
- **UART Current Sensing**: TX=GPIO_17, RX=GPIO_16 (to Arduino Nano)
- **WiFi**: 2.4GHz for web interface access
- **WiFi Mode Button**: GPIO_0 (Boot button) - physical switch between AP/Station modes

### PWM Configuration
- **PWM Frequency**: 100kHz with 8-bit resolution (0-255 duty cycle)
- **Current Sensors**: ACS712 with 8.0A overcurrent threshold
- **Safety Features**: Emergency stop, current monitoring, PWM limits

### Communication
#### Traditional Projects:
- **UART**: 115200 baud rate for serial communication
- **Bluetooth**: "Pod1_Bluetooth" device name for wireless data transmission

#### Web Server Project (Enhanced Real-time Communication):
- **WiFi Dual Mode**: Supports both Station mode (connect to router) and AP mode (create hotspot)
- **Station Mode**: Connects to existing WiFi network (WIFI_SSID/WIFI_PASSWORD in GLOBALS.hpp)
- **AP Mode**: Creates hotspot "BLDC_Motor_Control" / "BlDcMoToR2024" at 192.168.4.1
- **Dynamic Switching**: Runtime WiFi mode switching with NVS persistence
- **HTTP REST API**: Real-time polling every 1 second for responsive multi-device sync
- **UART**: 115200 baud communication with Arduino Nano current sensor
- **Real-time Synchronization**: All devices automatically sync PWM, lights, and door states
- **Core Assignment**: Optimized dual-core task distribution
  - Core 0: Motor PWM generation, current sensing, individual hardware control (lights/doors)
  - Core 1: Web server, HTTP API, speed calculation, data collection, real-time updates

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
- WiFi credentials and mode settings
- NVS (Non-Volatile Storage) configuration for persistent settings

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
✅ **motor_control_webserver/** - **PRODUCTION READY & FULLY OPTIMIZED**
- ✅ **All compilation errors resolved** - Clean build with no warnings
- ✅ **Memory optimized** - 78% program storage, 13% RAM usage
- ✅ **Legacy code removed** - Clean, maintainable codebase
- ✅ **Hardware-accurate pins** - Matches proven BLDC_speed_Arduino hardware
- ✅ **Modular architecture** - Individual Light and SingleDoorLock objects
- ✅ **Real-time HTTP polling** - 1-second updates for responsive multi-device sync
- ✅ **Enhanced UI synchronization** - All buttons and sliders sync across devices
- ✅ **Visual feedback** - Button state changes with animations and logging

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

### Web Interface Features - **ENHANCED WITH REAL-TIME SYNC**
- **Real-time Motor Control**: PWM slider syncs across all devices instantly
- **Live Data Display**: Motor speeds (RPM), current consumption (A), system voltage (V)
- **Individual Hardware Control**: 2 independent lights + 4 individual door locks
- **Multi-device Synchronization**: All buttons, sliders sync in real-time (1-second polling)
- **Visual Feedback**: Button state changes with animations and console logging
- **System Monitoring**: WiFi signal, uptime, memory usage with enhanced status indicators
- **Safety Features**: Emergency stop with status indicator animation
- **Mobile Responsive**: Sticky navbar for easy access to controls while scrolling

### Deployment Instructions - **PRODUCTION READY**
1. **✅ Verified Libraries**: ArduinoJson v6.21.5, ESP32 Core v3.3.0 (tested & working)
2. **✅ Clean Compilation**: Code compiles with no errors or warnings
3. **✅ Hardware Verified**: Pin assignments match proven BLDC_speed_Arduino hardware
4. **Configure WiFi Mode**: 
   - **Station Mode**: Edit WIFI_SSID and WIFI_PASSWORD in GLOBALS.hpp, set DEFAULT_USE_AP_MODE = false
   - **AP Mode**: Set DEFAULT_USE_AP_MODE = true for field operation (creates hotspot)
5. **Upload Web Files**: Use ESP32 LittleFS Data Upload tool for data/ folder
6. **Upload Arduino Sketch**: Standard upload process to ESP32
7. **Access Web Interface**: 
   - **Station Mode**: Navigate to assigned IP address (shown in serial monitor)
   - **AP Mode**: Connect to "BLDC_Motor_Control" WiFi, then go to 192.168.4.1
8. **Runtime WiFi Switching**: Press GPIO_0 button to switch between AP/Station modes (persisted in NVS)
9. **Multi-device Testing**: Open interface on multiple devices to verify real-time sync
10. **Optional Current Monitoring**: Upload current_sensor_nano.ino to Arduino Nano