#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "esp_log.h"

class WebServerManager {
private:
    WebServer* server;
    bool initialized;
    uint16_t port;
    
    // Static file handling
    void setupStaticRoutes();
    
    // API endpoints
    void setupApiRoutes();
    
    // Error handling
    void handleNotFound();
    
public:
    WebServerManager(uint16_t port = 80);
    ~WebServerManager();
    
    // Core functionality
    bool begin();
    void end();
    bool isRunning() const { return initialized; }
    
    // File system management
    bool initFileSystem();
    
    // CORS support
    void enableCORS();
    
    // Server instance access
    WebServer* getServer() { return server; }
    
    // WiFi management
    bool connectWiFi(const char* ssid, const char* password, unsigned long timeout = 30000);
    String getLocalIP();
};

WebServerManager::WebServerManager(uint16_t port) 
    : server(new WebServer(port)), initialized(false), port(port) {
}

WebServerManager::~WebServerManager() {
    end();
    delete server;
}

bool WebServerManager::begin() {
    if (initialized) return true;
    
    const char* TAG = "WebServerManager::begin";
    
    if (!initFileSystem()) {
        ESP_LOGE(TAG, "Failed to initialize LittleFS");
        return false;
    }
    
    setupStaticRoutes();
    setupApiRoutes();
    enableCORS();
    
    // Handle 404 errors
    server->onNotFound([this]() {
        handleNotFound();
    });
    
    server->begin();
    initialized = true;
    
    ESP_LOGI(TAG, "Web server started on port %d", port);
    return true;
}

void WebServerManager::end() {
    if (initialized) {
        server->stop();  // WebServer uses stop() instead of end()
        initialized = false;
        ESP_LOGI("WebServerManager", "Web server stopped");
    }
}

bool WebServerManager::initFileSystem() {
    return LittleFS.begin(true);
}

void WebServerManager::setupStaticRoutes() {
    // Serve root page
    server->on("/", HTTP_GET, [this]() {
        if (LittleFS.exists("/index.html")) {
            File file = LittleFS.open("/index.html", "r");
            server->streamFile(file, "text/html");
            file.close();
        } else {
            // Temporary: serve a basic test page until LittleFS files are uploaded
            String html = "<!DOCTYPE html><html><head><title>Motor Control System</title></head><body>";
            html += "<h1>ESP32 Motor Control System</h1>";
            html += "<p>✅ Web server is running!</p>";
            html += "<p>⚠️ Upload web files to LittleFS to see the full interface.</p>";
            html += "<p><strong>API Test Links:</strong></p>";
            html += "<ul>";
            html += "<li><a href='/api/system'>/api/system</a> - System info</li>";
            html += "<li><a href='/api/motor/status'>/api/motor/status</a> - Motor status</li>";
            html += "<li><a href='/api/wifi'>/api/wifi</a> - WiFi info</li>";
            html += "</ul>";
            html += "<p>Motor control via API only (no web interface yet)</p>";
            html += "</body></html>";
            server->send(200, "text/html", html);
        }
    });
    
    // Note: Static file serving will be handled in the global onNotFound handler
}

void WebServerManager::setupApiRoutes() {
    // System info endpoint
    server->on("/api/system", HTTP_GET, [this]() {
        DynamicJsonDocument doc(1024);
        
        doc["heap"] = ESP.getFreeHeap();
        doc["uptime"] = millis();
        doc["chip_model"] = ESP.getChipModel();
        doc["chip_revision"] = ESP.getChipRevision();
        doc["flash_size"] = ESP.getFlashChipSize();
        doc["wifi_rssi"] = WiFi.RSSI();
        doc["local_ip"] = WiFi.localIP().toString();
        
        String response;
        serializeJson(doc, response);
        
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->send(200, "application/json", response);
    });
    
    // WiFi info endpoint
    server->on("/api/wifi", HTTP_GET, [this]() {
        DynamicJsonDocument doc(1024);
        
        doc["ssid"] = WiFi.SSID();
        doc["rssi"] = WiFi.RSSI();
        doc["local_ip"] = WiFi.localIP().toString();
        doc["mac_address"] = WiFi.macAddress();
        doc["status"] = WiFi.status();
        
        String response;
        serializeJson(doc, response);
        
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->send(200, "application/json", response);
    });
    
    // Restart endpoint
    server->on("/api/restart", HTTP_POST, [this]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->send(200, "text/plain", "Restarting...");
        delay(1000);
        ESP.restart();
    });
    
    // File upload endpoint for drag-and-drop functionality
    server->on("/upload", HTTP_POST, [this]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->send(200, "text/plain", "File upload complete");
    }, [this]() {
        // Handle file upload
        HTTPUpload& upload = server->upload();
        static File uploadFile;
        
        if (upload.status == UPLOAD_FILE_START) {
            String filename = "/" + upload.filename;
            ESP_LOGI("WebServerManager", "Upload start: %s", filename.c_str());
            
            // Always overwrite existing files
            if (LittleFS.exists(filename)) {
                LittleFS.remove(filename);
            }
            
            uploadFile = LittleFS.open(filename, "w");
            if (!uploadFile) {
                ESP_LOGE("WebServerManager", "Failed to create file: %s", filename.c_str());
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (uploadFile) {
                uploadFile.write(upload.buf, upload.currentSize);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (uploadFile) {
                uploadFile.close();
                ESP_LOGI("WebServerManager", "Upload complete: %s (%d bytes)", upload.filename.c_str(), upload.totalSize);
            }
        }
    });
    
    // File management endpoint
    server->on("/api/files", HTTP_GET, [this]() {
        DynamicJsonDocument doc(1024);
        JsonArray files = doc.createNestedArray("files");
        
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file) {
            JsonObject fileObj = files.createNestedObject();
            fileObj["name"] = file.name();
            fileObj["size"] = file.size();
            file = root.openNextFile();
        }
        
        String response;
        serializeJson(doc, response);
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->send(200, "application/json", response);
    });
    
    // Delete file endpoint
    server->on("/api/files/delete", HTTP_POST, [this]() {
        if (server->hasArg("filename")) {
            String filename = server->arg("filename");
            if (filename.startsWith("/")) {
                filename = filename.substring(1);
            }
            filename = "/" + filename;
            
            if (LittleFS.exists(filename)) {
                LittleFS.remove(filename);
                server->sendHeader("Access-Control-Allow-Origin", "*");
                server->send(200, "application/json", "{\"status\":\"deleted\",\"file\":\"" + filename + "\"}");
            } else {
                server->sendHeader("Access-Control-Allow-Origin", "*");
                server->send(404, "application/json", "{\"error\":\"File not found\"}");
            }
        } else {
            server->sendHeader("Access-Control-Allow-Origin", "*");
            server->send(400, "application/json", "{\"error\":\"No filename provided\"}");
        }
    });
}

void WebServerManager::enableCORS() {
    // CORS headers will be added in each API response since WebServer doesn't have DefaultHeaders
    // Handle preflight OPTIONS requests for API routes
    server->on("/api/system", HTTP_OPTIONS, [this]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        server->send(200);
    });
    
    server->on("/api/wifi", HTTP_OPTIONS, [this]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        server->send(200);
    });
    
    server->on("/api/restart", HTTP_OPTIONS, [this]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        server->send(200);
    });
    
    // CORS for file upload endpoints
    server->on("/upload", HTTP_OPTIONS, [this]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        server->send(200);
    });
    
    server->on("/api/files", HTTP_OPTIONS, [this]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        server->send(200);
    });
    
    server->on("/api/files/delete", HTTP_OPTIONS, [this]() {
        server->sendHeader("Access-Control-Allow-Origin", "*");
        server->sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        server->sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        server->send(200);
    });
}

void WebServerManager::handleNotFound() {
    String path = server->uri();
    
    // Try to serve static files first
    if (server->method() == HTTP_GET) {
        if (path.endsWith("/")) {
            path += "index.html";
        }
        
        String contentType = "text/plain";
        if (path.endsWith(".html")) contentType = "text/html";
        else if (path.endsWith(".css")) contentType = "text/css";
        else if (path.endsWith(".js")) contentType = "application/javascript";
        else if (path.endsWith(".json")) contentType = "application/json";
        else if (path.endsWith(".ico")) contentType = "image/x-icon";
        
        if (LittleFS.exists(path)) {
            File file = LittleFS.open(path, "r");
            server->streamFile(file, contentType);
            file.close();
            return;
        }
    }
    
    // If file not found, return 404
    String message = "File Not Found\n\n";
    message += "URI: " + server->uri() + "\n";
    message += "Method: " + String((server->method() == HTTP_GET) ? "GET" : "POST") + "\n";
    message += "Arguments: " + String(server->args()) + "\n";
    
    for (uint8_t i = 0; i < server->args(); i++) {
        message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
    }
    
    ESP_LOGW("WebServerManager", "404 - %s", server->uri().c_str());
    server->send(404, "text/plain", message);
}

bool WebServerManager::connectWiFi(const char* ssid, const char* password, unsigned long timeout) {
    const char* TAG = "WebServerManager::connectWiFi";
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    ESP_LOGI(TAG, "Connecting to WiFi SSID: %s", ssid);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        delay(500);
        ESP_LOGI(TAG, ".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        ESP_LOGI(TAG, "WiFi connected!");
        ESP_LOGI(TAG, "IP address: %s", WiFi.localIP().toString().c_str());
        ESP_LOGI(TAG, "RSSI: %d dBm", WiFi.RSSI());
        return true;
    } else {
        ESP_LOGE(TAG, "WiFi connection failed!");
        return false;
    }
}

String WebServerManager::getLocalIP() {
    return WiFi.localIP().toString();
}