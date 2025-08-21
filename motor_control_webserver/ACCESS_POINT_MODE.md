# ESP32 Access Point Mode - Field Operation Guide

## 🌐 Overview

The ESP32 motor control system now supports **Access Point (AP) mode** for remote field operations where no existing WiFi network is available. In this mode, the ESP32 creates its own WiFi hotspot that you can connect to directly.

## 🔧 Configuration

### **Quick Setup**
Set `USE_AP_MODE = true` in `GLOBALS.hpp` to enable field operation mode.

### **Access Point Settings** (in GLOBALS.hpp)
```cpp
// WiFi Access Point Configuration
const char* AP_SSID = "BLDC_Motor_Control";        // Network name
const char* AP_PASSWORD = "BlDcMoToR2024";          // Password (min 8 chars)
const IPAddress AP_IP(192, 168, 4, 1);             // ESP32 IP address
const IPAddress AP_GATEWAY(192, 168, 4, 1);        // Gateway IP
const IPAddress AP_SUBNET(255, 255, 255, 0);       // Subnet mask

// Mode Selection
const bool USE_AP_MODE = true;  // true = Field Mode, false = Home/Office Mode
```

## 📱 How to Connect

### **Step 1: Power On ESP32**
- ESP32 starts and creates WiFi hotspot
- Serial monitor shows connection details

### **Step 2: Connect Your Device**
1. **Phone/Tablet/Laptop**: Open WiFi settings
2. **Find Network**: Look for `BLDC_Motor_Control`
3. **Enter Password**: `BlDcMoToR2024`
4. **Connect**: Your device will connect to ESP32

### **Step 3: Access Web Interface**
1. **Open Browser**: Chrome, Safari, Firefox, etc.
2. **Go to**: `http://192.168.4.1`
3. **Control Motors**: Full web interface available

## 🏃‍♂️ Field Operation Benefits

### **✅ Advantages**
- **No Internet Required**: Works anywhere, anytime
- **Direct Connection**: No router or WiFi infrastructure needed
- **Multiple Devices**: Up to 4 devices can connect simultaneously
- **Low Latency**: Direct communication with ESP32
- **Portable**: Perfect for remote locations, workshops, field testing

### **📋 Typical Use Cases**
- Motor testing in remote locations
- Field commissioning and setup
- Workshop demonstrations
- Outdoor projects without WiFi
- Emergency or backup operation

## 🔄 Switching Between Modes

### **Field Mode (AP Mode)**
```cpp
const bool USE_AP_MODE = true;
```
- ESP32 creates hotspot `BLDC_Motor_Control`
- Access via `http://192.168.4.1`
- Perfect for field operation

### **Home/Office Mode (Station Mode)**
```cpp
const bool USE_AP_MODE = false;
```
- Connects to existing WiFi network
- Access via assigned IP (shown in serial monitor)
- Perfect for permanent installations

## 📺 Arduino Nano OLED Display

The Arduino Nano OLED will automatically display the ESP32's IP address:

**Field Mode Display:**
```
I1: 2.50A
I2: 1.75A
V:  12.5V
ESP32:
192.168.4.1
```

## 🛠️ Technical Specifications

### **Network Configuration**
- **SSID**: BLDC_Motor_Control
- **Security**: WPA2-PSK
- **IP Range**: 192.168.4.1 - 192.168.4.254
- **DHCP**: Enabled (devices get IP automatically)
- **Max Clients**: 4 simultaneous connections

### **Web Interface Features**
- ✅ Real-time motor control and monitoring
- ✅ Individual door lock control
- ✅ LED lighting control
- ✅ Current sensing display
- ✅ System status monitoring
- ✅ Emergency stop functionality

## 🔧 Troubleshooting

### **Can't Find WiFi Network**
- Check `USE_AP_MODE = true` in GLOBALS.hpp
- Verify ESP32 is powered and running
- Look for `BLDC_Motor_Control` in WiFi list
- Try refreshing WiFi scan on your device

### **Can't Connect to Network**
- Verify password: `BlDcMoToR2024`
- Check if 4 devices are already connected (max limit)
- Restart ESP32 and try again

### **Can't Access Web Interface**
- Ensure you're connected to `BLDC_Motor_Control` WiFi
- Try `http://192.168.4.1` in browser
- Clear browser cache and try again
- Check if device received IP address (should be 192.168.4.x)

### **Arduino Nano Not Showing IP**
- Check UART connection (ESP32 GPIO 17/16 to Arduino)
- Verify Arduino Nano code is uploaded
- Check serial monitor for IP transmission messages

## 🚀 Getting Started Checklist

- [ ] Set `USE_AP_MODE = true` in GLOBALS.hpp
- [ ] Upload ESP32 code
- [ ] Upload Arduino Nano current sensor code
- [ ] Power on system
- [ ] Connect phone/laptop to `BLDC_Motor_Control` WiFi
- [ ] Open browser to `http://192.168.4.1`
- [ ] Verify OLED shows ESP32 IP address
- [ ] Test motor controls and monitoring

## 📊 Performance

**Connection Time**: ~10-15 seconds for AP to start
**Web Page Load**: <2 seconds over direct connection
**Real-time Updates**: 1-second polling interval
**Range**: ~50-100 meters (depending on environment)
**Stability**: Excellent for field operation (no internet dependencies)

---

**Perfect for field engineers, technicians, and remote motor testing! 🏗️⚡**