#include "GLOBALS.hpp"

// Object Instantiation
PwmGenerator motorPWM1(motorPwmPin1, frequency, resolution);
PwmGenerator motorPWM2(motorPwmPin2, frequency, resolution);
BLDCPulseCalculator motorPulse1(feedBackPin1, motorId1);
BLDCPulseCalculator motorPulse2(feedBackPin2, motorId2);
MotorDirection direction;
DoorLock doorLock(doorLock1Pin, doorLock2Pin, doorLock3Pin, doorLock4Pin);
UARTCurrentSensor currentSensor;

WebServerManager webServer;
DataCollector dataCollector;
ControlInterface controlInterface;

// Task Handles
TaskHandle_t motor1SpeedTaskHandle = nullptr;
TaskHandle_t motor2SpeedTaskHandle = nullptr;
TaskHandle_t motor1PWMTaskHandle = nullptr;
TaskHandle_t motor2PWMTaskHandle = nullptr;
TaskHandle_t currentSensorTaskHandle = nullptr;
TaskHandle_t dataCollectorTaskHandle = nullptr;
TaskHandle_t webSocketTaskHandle = nullptr;

void addMotorControlAPI() {
    WebServer* server = webServer.getServer();
    
    // Motor status endpoint - replaces WebSocket status updates
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
            
            if (doc.containsKey("lights")) {
                bool lights = doc["lights"];
                gpio_set_level(lightsPin, lights ? 1 : 0);
                lightsOn = lights;
            }
            
            if (doc.containsKey("doors")) {
                JsonObject doors = doc["doors"];
                if (doors.containsKey("lock_all")) {
                    bool lockAll = doors["lock_all"];
                    if (lockAll) {
                        doorLock.lockAllDoors();
                    } else {
                        doorLock.unlockAllDoors();
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
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== Motor Control WebServer Starting ===");
    
    // Initialize LittleFS for web files
    if(!LittleFS.begin(true)) {
        Serial.println("ERROR: LittleFS Mount Failed");
        return;
    }
    Serial.println("✓ LittleFS initialized");
    
    // Initialize WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to WiFi");
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.print("✓ WiFi connected! IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n✗ WiFi connection failed!");
        return;
    }
    
    // Initialize GPIO for lights control
    gpio_config_t gpioLightsConfig = {
        .pin_bit_mask = (1ULL << lightsPin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&gpioLightsConfig);
    gpio_set_level(lightsPin, 0);
    Serial.println("✓ Lights GPIO initialized");
    
    // Initialize PWM Generators (Core 0 - Motor Control)
    motorPWM1.begin(motor1PWMTaskHandle, app_cpu0);
    motorPWM2.begin(motor2PWMTaskHandle, app_cpu0);
    Serial.println("✓ PWM generators initialized on Core 0");
    
    // Initialize Speed Calculators (Core 1 - Processing)
    motorPulse1.begin(motor1SpeedTaskHandle, app_cpu1);
    motorPulse2.begin(motor2SpeedTaskHandle, app_cpu1);
    Serial.println("✓ Speed calculators initialized on Core 1");
    
    // Initialize Door Lock System (Core 0 - Hardware Control)
    doorLock.begin();
    Serial.println("✓ Door lock system initialized");
    
    // Initialize Current Sensor UART Communication (Core 0 - Hardware)
    currentSensor.begin(currentSensorTaskHandle, app_cpu0);
    Serial.println("✓ Current sensor UART initialized on Core 0");
    
    // Wait for network stack to fully initialize
    delay(1000);
    
    // Initialize Web Server (Core 1 - Network) - BEFORE other components
    if (!webServer.begin()) {
        Serial.println("✗ Failed to start web server");
        return;
    }
    Serial.println("✓ Web server initialized on Core 1");
    
    // Add delay before WebSocket initialization to ensure TCPIP stack is ready
    delay(500);
    
    // WebSocket functionality replaced with simple HTTP polling
    Serial.println("✓ Using HTTP polling instead of WebSocket for better stability");
    
    // Initialize Data Collector (Core 1 - Data Processing) - AFTER network setup
    dataCollector.begin();
    Serial.println("✓ Data collector initialized on Core 1");
    
    // Initialize Control Interface
    controlInterface.begin();
    Serial.println("✓ Control interface initialized");
    
    // Add motor control API endpoints
    addMotorControlAPI();
    Serial.println("✓ Motor control API endpoints added");
    
    Serial.println("=== System Initialization Complete ===");
    Serial.print("Web Interface: http://");
    Serial.println(WiFi.localIP());
    Serial.println("==========================================");
    
    // Print memory status
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
}

void loop() {
    // Handle HTTP requests - CRITICAL for synchronous WebServer
    webServer.getServer()->handleClient();
    
    // Check WiFi connection and attempt reconnection if needed
    static unsigned long lastWiFiCheck = 0;
    if (millis() - lastWiFiCheck > 30000) { // Check every 30 seconds
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected, attempting reconnection...");
            WiFi.reconnect();
        }
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