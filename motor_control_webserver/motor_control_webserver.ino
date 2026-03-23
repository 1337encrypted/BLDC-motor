#include "GLOBALS.hpp"

// Global variables for runtime mode switching
bool currentUseAPMode = DEFAULT_USE_AP_MODE;
Preferences preferences;

// Function declarations
void loadWiFiMode();
void saveWiFiMode(bool useAPMode);
void switchWiFiMode(bool useAPMode);
void sendWiFiModeToArduino(bool useAPMode, const String& ip);
void setupWiFiModeButton();
void checkWiFiModeButton();

// Object Instantiation
PwmGenerator motorPWM1(motorPwmPin1, frequency, resolution);
PwmGenerator motorPWM2(motorPwmPin2, frequency, resolution);
BLDCPulseCalculator motorPulse1(feedBackPin1, motorId1);
BLDCPulseCalculator motorPulse2(feedBackPin2, motorId2);
MotorDirection direction;
UARTCurrentSensor currentSensor;

// New Modular Objects
Light leftLight(leftLightPin, "Left Light");
Light rightLight(rightLightPin, "Right Light");
SingleDoorLock frontLeftDoor(doorLock1Pin, DoorPosition::FRONT_LEFT, "Front Left Door");
SingleDoorLock frontRightDoor(doorLock2Pin, DoorPosition::FRONT_RIGHT, "Front Right Door");
SingleDoorLock rearLeftDoor(doorLock3Pin, DoorPosition::REAR_LEFT, "Rear Left Door");
SingleDoorLock rearRightDoor(doorLock4Pin, DoorPosition::REAR_RIGHT, "Rear Right Door");

WebServerManager webServer;
DataCollector dataCollector;

// Task Handles
TaskHandle_t motor1SpeedTaskHandle = nullptr;
TaskHandle_t motor2SpeedTaskHandle = nullptr;
TaskHandle_t motor1PWMTaskHandle = nullptr;
TaskHandle_t motor2PWMTaskHandle = nullptr;
TaskHandle_t currentSensorTaskHandle = nullptr;
TaskHandle_t dataCollectorTaskHandle = nullptr;

// WiFi Mode Management Functions
void loadWiFiMode() {
    preferences.begin("motor_config", false);  // false = read/write mode
    currentUseAPMode = preferences.getBool("useAPMode", DEFAULT_USE_AP_MODE);
    preferences.end();
    
    Serial.printf("📂 Loaded WiFi mode from NVS: %s\n", 
                  currentUseAPMode ? "AP Mode (Field)" : "Station Mode (Home/Office)");
}

void saveWiFiMode(bool useAPMode) {
    preferences.begin("motor_config", false);
    preferences.putBool("useAPMode", useAPMode);
    preferences.end();
    
    Serial.printf("💾 Saved WiFi mode to NVS: %s\n", 
                  useAPMode ? "AP Mode (Field)" : "Station Mode (Home/Office)");
}

void switchWiFiMode(bool useAPMode) {
    if (currentUseAPMode == useAPMode) {
        Serial.println("🔄 WiFi mode already set to requested mode");
        return;
    }
    
    Serial.printf("🔄 Switching WiFi mode: %s -> %s\n",
                  currentUseAPMode ? "AP Mode" : "Station Mode",
                  useAPMode ? "AP Mode" : "Station Mode");
    
    // Save new mode to NVS
    saveWiFiMode(useAPMode);
    currentUseAPMode = useAPMode;
    
    // Send mode change to Arduino
    String currentIP = WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
    sendWiFiModeToArduino(useAPMode, currentIP);
    
    Serial.println("⚠️  WiFi mode changed! System will restart in 3 seconds...");
    delay(3000);
    ESP.restart();  // Restart to apply new WiFi mode
}

void sendWiFiModeToArduino(bool useAPMode, const String& ip) {
    // Send IP address first
    currentSensor.sendIPAddress(ip);
    
    // Small delay then send WiFi mode info
    delay(200);
    currentSensor.sendWiFiMode(useAPMode, ip);
    
    Serial.printf("📡 Sent mode info to Arduino: %s, IP: %s\n", 
                  useAPMode ? "AP Mode (Field)" : "Station Mode (Home/Office)", ip.c_str());
}

// Physical button setup and handling
void setupWiFiModeButton() {
    pinMode(WIFI_MODE_BUTTON_PIN, INPUT_PULLUP);
    Serial.println("🔘 WiFi mode button initialized (GPIO 0 - Boot button)");
    Serial.println("💡 Hold button for 3 seconds to toggle WiFi mode");
}

void checkWiFiModeButton() {
    static unsigned long buttonPressStart = 0;
    static bool buttonPressed = false;
    static bool buttonProcessed = false;
    
    bool currentButtonState = (digitalRead(WIFI_MODE_BUTTON_PIN) == LOW);
    
    if (currentButtonState && !buttonPressed) {
        // Button just pressed
        buttonPressed = true;
        buttonProcessed = false;
        buttonPressStart = millis();
        Serial.println("🔘 WiFi mode button pressed...");
        
    } else if (!currentButtonState && buttonPressed) {
        // Button just released
        buttonPressed = false;
        unsigned long pressDuration = millis() - buttonPressStart;
        
        if (pressDuration >= 3000 && !buttonProcessed) {
            // Long press detected - toggle WiFi mode
            Serial.println("🔄 Long press detected! Toggling WiFi mode...");
            switchWiFiMode(!currentUseAPMode);
        } else if (pressDuration < 3000) {
            Serial.printf("🔘 Short press (%lu ms) - Hold for 3+ seconds to switch mode\n", pressDuration);
        }
        
    } else if (currentButtonState && buttonPressed) {
        // Button is being held
        unsigned long pressDuration = millis() - buttonPressStart;
        
        if (pressDuration >= 3000 && !buttonProcessed) {
            // Long press threshold reached
            buttonProcessed = true;
            Serial.println("🎯 Long press confirmed! Release button to toggle WiFi mode...");
        }
    }
}

void addMotorControlAPI() {
    WebServer* server = webServer.getServer();
    
    // Motor status endpoint - provides real-time data via HTTP polling
    server->on("/api/motor/status", HTTP_GET, [server]() {
        DynamicJsonDocument doc = dataCollector.getAllData();
        String response;
        serializeJson(doc, response);
        
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->send(200, "application/json", response);
    });
    
    // Motor control endpoint
    server->on("/api/motor/control", HTTP_POST, [server]() {
        if (server->hasArg("plain")) {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, server->arg("plain"));
            
            if (error) {
                server->sendHeader("Access-Control-Allow-Origin", "*");
                server->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            
            // Process motor control commands
            if (doc.containsKey("pwm")) {
                uint8_t pwm = doc["pwm"];
                motorPWM1.setPwm(pwm);
                motorPWM2.setPwm(pwm);
                currentPWM = pwm;
            }
            
            // Handle individual lights control
            if (doc.containsKey("lights")) {
                JsonVariant lightsVariant = doc["lights"];
                
                // Check if lights is a boolean (legacy support)
                if (lightsVariant.is<bool>()) {
                    bool lightState = lightsVariant.as<bool>();
                    leftLight.setState(lightState);
                    rightLight.setState(lightState);
                    leftLightOn = rightLightOn = lightState;
                }
                // Handle as object (new individual control)
                else if (lightsVariant.is<JsonObject>()) {
                    JsonObject lights = lightsVariant.as<JsonObject>();
                    
                    if (lights.containsKey("left")) {
                        String leftAction = lights["left"];
                        if (leftAction == "toggle") {
                            leftLight.toggle();
                            leftLightOn = leftLight.isOn();
                        } else if (leftAction == "on") {
                            leftLight.turnOn();
                            leftLightOn = true;
                        } else if (leftAction == "off") {
                            leftLight.turnOff();
                            leftLightOn = false;
                        }
                    }
                    if (lights.containsKey("right")) {
                        String rightAction = lights["right"];
                        if (rightAction == "toggle") {
                            rightLight.toggle();
                            rightLightOn = rightLight.isOn();
                        } else if (rightAction == "on") {
                            rightLight.turnOn();
                            rightLightOn = true;
                        } else if (rightAction == "off") {
                            rightLight.turnOff();
                            rightLightOn = false;
                        }
                    }
                }
            }
            
            // Handle individual doors control
            if (doc.containsKey("doors")) {
                JsonObject doors = doc["doors"];
                
                // Individual door control
                if (doors.containsKey("left")) {
                    String leftAction = doors["left"];
                    if (leftAction == "toggle") {
                        frontLeftDoor.toggle();
                        door1Locked = frontLeftDoor.isLockLocked();
                    } else if (leftAction == "lock") {
                        frontLeftDoor.lock();
                        door1Locked = true;
                    } else if (leftAction == "unlock") {
                        frontLeftDoor.unlock();
                        door1Locked = false;
                    }
                }
                if (doors.containsKey("right")) {
                    String rightAction = doors["right"];
                    if (rightAction == "toggle") {
                        frontRightDoor.toggle();
                        door2Locked = frontRightDoor.isLockLocked();
                    } else if (rightAction == "lock") {
                        frontRightDoor.lock();
                        door2Locked = true;
                    } else if (rightAction == "unlock") {
                        frontRightDoor.unlock();
                        door2Locked = false;
                    }
                }
                
                // Legacy support for all doors
                if (doors.containsKey("lock_all")) {
                    bool lockAll = doors["lock_all"];
                    if (lockAll) {
                        frontLeftDoor.lock();
                        frontRightDoor.lock();
                        rearLeftDoor.lock();
                        rearRightDoor.lock();
                        door1Locked = door2Locked = door3Locked = door4Locked = true;
                        doorsLocked = true;
                    } else {
                        frontLeftDoor.unlock();
                        frontRightDoor.unlock();
                        rearLeftDoor.unlock();
                        rearRightDoor.unlock();
                        door1Locked = door2Locked = door3Locked = door4Locked = false;
                        doorsLocked = false;
                    }
                }
            }
            
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(200, "application/json", "{\"status\":\"ok\"}");
        } else {
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(400, "application/json", "{\"error\":\"No data received\"}");
        }
    });
    
    // CORS for motor endpoints
    server->on("/api/motor/status", HTTP_OPTIONS, [server]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server->send(200);
    });
    
    server->on("/api/motor/control", HTTP_OPTIONS, [server]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server->send(200);
    });
    
    // WiFi Mode Management API
    server->on("/api/wifi/mode", HTTP_GET, [server]() {
        DynamicJsonDocument doc(512);
        doc["currentMode"] = currentUseAPMode ? "AP" : "Station";
        doc["modeDescription"] = currentUseAPMode ? "Field Mode (Access Point)" : "Home/Office Mode (Station)";
        doc["canSwitch"] = true;
        doc["currentIP"] = WiFi.getMode() == WIFI_AP ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
        doc["ssid"] = currentUseAPMode ? AP_SSID : WIFI_SSID;
        
        String response;
        serializeJson(doc, response);
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->send(200, "application/json", response);
    });
    
    server->on("/api/wifi/switch", HTTP_POST, [server]() {
        if (server->hasArg("plain")) {
            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, server->arg("plain"));
            
            if (!error && doc.containsKey("mode")) {
                String newMode = doc["mode"];
                bool useAPMode = (newMode == "AP");
                
                // Send immediate response before restart
                server->sendHeader("Access-Control-Allow-Origin", "*");
                server->send(200, "application/json", "{\"status\":\"switching\",\"message\":\"WiFi mode changing, system will restart\"}");
                
                // Switch mode (this will restart the system)
                delay(100);  // Allow response to be sent
                switchWiFiMode(useAPMode);
            } else {
                server->sendHeader("Access-Control-Allow-Origin", "*");
                server->send(400, "application/json", "{\"error\":\"Invalid mode specified. Use 'AP' or 'Station'\"}");
            }
        } else {
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(400, "application/json", "{\"error\":\"No data received\"}");
        }
    });
    
    // CORS for WiFi endpoints
    server->on("/api/wifi/mode", HTTP_OPTIONS, [server]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server->send(200);
    });
    
    server->on("/api/wifi/switch", HTTP_OPTIONS, [server]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server->send(200);
    });
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Motor Control WebServer Starting ===");
    
    // Load WiFi mode from NVS (Non-Volatile Storage)
    loadWiFiMode();
    
    // Setup physical WiFi mode button
    setupWiFiModeButton();
    
    // Initialize LittleFS for web files
    if(!LittleFS.begin(true)) {
        Serial.println("ERROR: LittleFS Mount Failed");
        return;
    }
    Serial.println("✓ LittleFS initialized");
    
    // Initialize WiFi based on loaded mode
    String localIP;
    if (currentUseAPMode) {
        // Access Point Mode - ESP32 becomes a hotspot
        Serial.println("🌐 Starting WiFi Access Point mode...");
        Serial.printf("SSID: %s\n", AP_SSID);
        Serial.printf("Password: %s\n", AP_PASSWORD);
        
        if (webServer.startAccessPoint(AP_SSID, AP_PASSWORD, AP_IP, AP_GATEWAY, AP_SUBNET)) {
            localIP = WiFi.softAPIP().toString();
            Serial.println("✅ Access Point started successfully!");
            Serial.printf("📱 Connect to WiFi: %s\n", AP_SSID);
            Serial.printf("🔒 Password: %s\n", AP_PASSWORD);
            Serial.printf("🌐 Web Interface: http://%s\n", localIP.c_str());
            Serial.println("📍 Perfect for field operation - no internet required!");
        } else {
            Serial.println("❌ Failed to start Access Point!");
            return;
        }
    } else {
        // Station Mode - Connect to existing WiFi network
        Serial.println("🔗 Connecting to existing WiFi network...");
        Serial.printf("SSID: %s\n", WIFI_SSID);
        
        if (webServer.connectWiFi(WIFI_SSID, WIFI_PASSWORD, 30000)) {
            localIP = WiFi.localIP().toString();
            Serial.println("✅ WiFi connected successfully!");
            Serial.printf("🌐 Web Interface: http://%s\n", localIP.c_str());
        } else {
            Serial.println("❌ WiFi connection failed!");
            Serial.println("🔄 Automatically switching to AP mode for field operation...");
            Serial.println("📝 This change will be remembered for next boot");
            
            // Switch to AP mode automatically and save to NVS
            saveWiFiMode(true);  // Save AP mode to NVS for persistence
            currentUseAPMode = true;
            
            // Start AP mode immediately
            Serial.println("🌐 Starting WiFi Access Point mode...");
            Serial.printf("SSID: %s\n", AP_SSID);
            Serial.printf("Password: %s\n", AP_PASSWORD);
            
            if (webServer.startAccessPoint(AP_SSID, AP_PASSWORD, AP_IP, AP_GATEWAY, AP_SUBNET)) {
                localIP = WiFi.softAPIP().toString();
                Serial.println("✅ Access Point started successfully!");
                Serial.printf("📱 Connect to WiFi: %s\n", AP_SSID);
                Serial.printf("🔒 Password: %s\n", AP_PASSWORD);
                Serial.printf("🌐 Web Interface: http://%s\n", localIP.c_str());
                Serial.println("📍 Perfect for field operation - no internet required!");
            } else {
                Serial.println("❌ Failed to start Access Point! System halted.");
                return;
            }
        }
    }
    
    // Initialize individual lights
    leftLight.begin();
    rightLight.begin();
    Serial.println("✓ Individual lights initialized");
    
    // Initialize PWM Generators (Core 0 - Motor Control)
    motorPWM1.begin(motor1PWMTaskHandle, app_cpu0);
    motorPWM2.begin(motor2PWMTaskHandle, app_cpu0);
    Serial.println("✓ PWM generators initialized on Core 0");
    
    // Initialize Speed Calculators (Core 1 - Processing)
    motorPulse1.begin(motor1SpeedTaskHandle, app_cpu1);
    motorPulse2.begin(motor2SpeedTaskHandle, app_cpu1);
    Serial.println("✓ Speed calculators initialized on Core 1");
    
    // Initialize individual door locks
    frontLeftDoor.begin();
    frontRightDoor.begin();
    rearLeftDoor.begin();
    rearRightDoor.begin();
    Serial.println("✓ Individual door lock system initialized");
    
    // Initialize Current Sensor UART Communication (Core 0 - Hardware)
    currentSensor.begin(currentSensorTaskHandle, app_cpu0);
    Serial.println("✓ Current sensor UART initialized on Core 0");
    
    // Send IP address to Arduino Nano for OLED display (once after initialization)
    if (!localIP.isEmpty() && localIP != "0.0.0.0") {
        Serial.printf("📡 Transmitting IP address to Arduino Nano: %s\n", localIP.c_str());
        
        // Allow UART task to start
        delay(500);
        
        // Send IP address via UART
        currentSensor.sendIPAddress(localIP);
        Serial.println("✓ IP address sent to Arduino Nano OLED");
    }
    
    // Wait for network stack to fully initialize
    delay(1000);
    
    // Initialize Web Server (Core 1 - Network) - BEFORE other components
    if (!webServer.begin()) {
        Serial.println("✗ Failed to start web server");
        return;
    }
    Serial.println("✓ Web server initialized on Core 1");
    
    // Add delay to ensure network stack is ready
    delay(500);
    
    // Using HTTP polling for better stability and field operation reliability
    Serial.println("✓ HTTP polling enabled for real-time updates");
    
    // Initialize Data Collector (Core 1 - Data Processing) - AFTER network setup
    dataCollector.begin();
    Serial.println("✓ Data collector initialized on Core 1");
    
    
    // Add motor control API endpoints
    addMotorControlAPI();
    Serial.println("✓ Motor control API endpoints added");
    
    // Send initial mode and IP to Arduino Nano
    sendWiFiModeToArduino(currentUseAPMode, localIP);
    
    Serial.println("=== System Initialization Complete ===");
    if (currentUseAPMode) {
        Serial.printf("📱 WiFi Network: %s (Password: %s)\n", AP_SSID, AP_PASSWORD);
        Serial.printf("🌐 Web Interface: http://%s\n", localIP.c_str());
        Serial.println("📍 Field Operation Mode - Connect directly to ESP32");
        Serial.println("🔄 Switch to Home/Office mode via web interface");
    } else {
        Serial.printf("🌐 Web Interface: http://%s\n", localIP.c_str());
        Serial.println("🏠 Home/Office Mode - Connected to existing WiFi");
        Serial.println("🔄 Switch to Field mode via web interface");
    }
    Serial.println("==========================================");
    
    // Print memory status
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
}

void loop() {
    // Handle HTTP requests - CRITICAL for synchronous WebServer
    webServer.getServer()->handleClient();
    
    // Check for physical WiFi mode button presses
    checkWiFiModeButton();
    
    // Check WiFi connection and send status updates to Arduino
    static unsigned long lastWiFiCheck = 0;
    static int failedReconnectAttempts = 0;
    
    if (millis() - lastWiFiCheck > 10000) { // Check every 10 seconds
        if (!currentUseAPMode && WiFi.status() != WL_CONNECTED) {
            failedReconnectAttempts++;
            Serial.printf("WiFi disconnected (attempt %d/3), checking connection...\n", failedReconnectAttempts);
            
            if (failedReconnectAttempts <= 3) {
                WiFi.reconnect();
            } else {
                Serial.println("❌ Station mode failed after 3 attempts!");
                Serial.println("🔄 Automatically switching to AP mode for field operation...");
                switchWiFiMode(true); // Switch to AP mode and restart
                return;
            }
        } else if (!currentUseAPMode && WiFi.status() == WL_CONNECTED) {
            // Reset failure counter when successfully connected
            if (failedReconnectAttempts > 0) {
                Serial.println("✅ WiFi reconnected successfully!");
                failedReconnectAttempts = 0;
            }
        }
        
        // Send periodic WiFi strength to Arduino for OLED status bar
        int currentRSSI = currentUseAPMode ? -50 : WiFi.RSSI(); // Use fixed value for AP mode
        currentSensor.sendWiFiStrength(currentRSSI);
        
        lastWiFiCheck = millis();
    }
    
    // Watchdog feed and memory monitoring
    static unsigned long lastMemoryCheck = 0;
    if (millis() - lastMemoryCheck > 60000) { // Check every minute
        uint32_t freeHeap = ESP.getFreeHeap();
        if (freeHeap < 10000) { // Less than 10KB free
            Serial.printf("WARNING: Low memory! Free heap: %d bytes\n", freeHeap);
        }
        lastMemoryCheck = millis();
        
        // Print system status
        Serial.printf("System Status - Heap: %dKB, Uptime: %ds\n", 
                     freeHeap/1024, millis()/1000);
    }
    
    // Emergency stop check (if needed)
    // This could monitor for hardware emergency stop button
    
    // Small delay to prevent watchdog issues but keep HTTP responsive
    delay(10);
}