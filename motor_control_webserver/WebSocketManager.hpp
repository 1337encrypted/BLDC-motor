#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <functional>
#include <map>
#include "esp_log.h"

struct WebSocketMessage {
    String type;
    DynamicJsonDocument data;
    uint32_t clientId;
    uint32_t timestamp;
    
    WebSocketMessage() : data(1024) {}
};

class WebSocketManager {
private:
    AsyncWebSocket* ws;
    std::map<String, std::function<void(const WebSocketMessage&)>> messageHandlers;
    std::map<uint32_t, String> clientInfo;
    String wsPath;
    
    // Event handlers
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                 AwsEventType type, void *arg, uint8_t *data, size_t len);
    void handleMessage(AsyncWebSocketClient *client, uint8_t *data, size_t len);
    
    // Connection management
    void onClientConnect(AsyncWebSocketClient *client);
    void onClientDisconnect(AsyncWebSocketClient *client);
    
public:
    WebSocketManager(const String& path = "/ws");
    ~WebSocketManager();
    
    // Core functionality
    void begin(AsyncWebServer* server);
    void end();
    
    // Message handling
    void registerHandler(const String& messageType, 
                        std::function<void(const WebSocketMessage&)> handler);
    void removeHandler(const String& messageType);
    
    // Broadcasting
    void broadcast(const String& type, const JsonDocument& data);
    void broadcastToClient(uint32_t clientId, const String& type, const JsonDocument& data);
    void sendStatus(const String& status, const JsonDocument& data);
    void sendError(const String& error, uint32_t clientId = 0);
    
    // Client management
    size_t getClientCount() const;
    void cleanupClients(size_t maxClients = 10);
    bool isClientConnected(uint32_t clientId);
    void pingAllClients();
    
    // WebSocket instance access
    AsyncWebSocket* getWebSocket() { return ws; }
};

WebSocketManager::WebSocketManager(const String& path) 
    : ws(new AsyncWebSocket(path.c_str())), wsPath(path) {
}

WebSocketManager::~WebSocketManager() {
    end();
    delete ws;
}

void WebSocketManager::begin(AsyncWebServer* server) {
    const char* TAG = "WebSocketManager::begin";
    
    ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, 
                       void *arg, uint8_t *data, size_t len) {
        this->onEvent(server, client, type, arg, data, len);
    });
    
    server->addHandler(ws);
    ESP_LOGI(TAG, "WebSocket server started on path: %s", wsPath.c_str());
}

void WebSocketManager::end() {
    if (ws) {
        ws->closeAll();
        ESP_LOGI("WebSocketManager", "WebSocket server stopped");
    }
}

void WebSocketManager::onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                               AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            onClientConnect(client);
            break;
            
        case WS_EVT_DISCONNECT:
            onClientDisconnect(client);
            break;
            
        case WS_EVT_DATA: {
            AwsFrameInfo *info = (AwsFrameInfo*)arg;
            if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                handleMessage(client, data, len);
            }
            break;
        }
            
        case WS_EVT_PONG:
            ESP_LOGD("WebSocketManager", "WebSocket pong from client %u", client->id());
            break;
            
        case WS_EVT_ERROR:
            ESP_LOGW("WebSocketManager", "WebSocket error from client %u", client->id());
            break;
    }
}

void WebSocketManager::onClientConnect(AsyncWebSocketClient *client) {
    ESP_LOGI("WebSocketManager", "WebSocket client #%u connected from %s", 
             client->id(), client->remoteIP().toString().c_str());
    
    // Send welcome message with current system status
    DynamicJsonDocument welcomeData(512);
    welcomeData["status"] = "connected";
    welcomeData["clientId"] = client->id();
    welcomeData["serverTime"] = millis();
    welcomeData["version"] = "1.0.0";
    
    broadcastToClient(client->id(), "connection", welcomeData);
    
    // Clean up if too many clients
    if (getClientCount() > 10) {
        cleanupClients(8);
    }
}

void WebSocketManager::onClientDisconnect(AsyncWebSocketClient *client) {
    ESP_LOGI("WebSocketManager", "WebSocket client #%u disconnected", client->id());
    clientInfo.erase(client->id());
}

void WebSocketManager::handleMessage(AsyncWebSocketClient *client, uint8_t *data, size_t len) {
    // Create safe buffer with null termination
    char* buffer = new char[len + 1];
    memcpy(buffer, data, len);
    buffer[len] = 0;
    String message = buffer;
    delete[] buffer;
    
    ESP_LOGD("WebSocketManager", "Received message from client %u: %s", client->id(), message.c_str());
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        ESP_LOGW("WebSocketManager", "JSON parsing failed: %s", error.c_str());
        sendError("Invalid JSON format", client->id());
        return;
    }
    
    if (!doc.containsKey("type")) {
        ESP_LOGW("WebSocketManager", "Message missing 'type' field");
        sendError("Message missing 'type' field", client->id());
        return;
    }
    
    WebSocketMessage wsMessage;
    wsMessage.type = doc["type"].as<String>();
    wsMessage.data = doc;
    wsMessage.clientId = client->id();
    wsMessage.timestamp = millis();
    
    // Handle message based on type
    auto handler = messageHandlers.find(wsMessage.type);
    if (handler != messageHandlers.end()) {
        try {
            handler->second(wsMessage);
        } catch (const std::exception& e) {
            ESP_LOGE("WebSocketManager", "Handler error for type %s: %s", wsMessage.type.c_str(), e.what());
            sendError("Handler error", client->id());
        }
    } else {
        ESP_LOGW("WebSocketManager", "No handler for message type: %s", wsMessage.type.c_str());
        sendError("Unknown message type: " + wsMessage.type, client->id());
    }
}

void WebSocketManager::registerHandler(const String& messageType, 
                                       std::function<void(const WebSocketMessage&)> handler) {
    messageHandlers[messageType] = handler;
    ESP_LOGI("WebSocketManager", "Registered handler for message type: %s", messageType.c_str());
}

void WebSocketManager::removeHandler(const String& messageType) {
    messageHandlers.erase(messageType);
    ESP_LOGI("WebSocketManager", "Removed handler for message type: %s", messageType.c_str());
}

void WebSocketManager::broadcast(const String& type, const JsonDocument& data) {
    DynamicJsonDocument doc(1024);
    doc["type"] = type;
    doc["data"] = data;
    doc["timestamp"] = millis();
    
    String message;
    serializeJson(doc, message);
    ws->textAll(message);
    
    ESP_LOGD("WebSocketManager", "Broadcast message type: %s to %d clients", type.c_str(), getClientCount());
}

void WebSocketManager::broadcastToClient(uint32_t clientId, const String& type, const JsonDocument& data) {
    auto client = ws->client(clientId);
    if (client && client->status() == WS_CONNECTED) {
        DynamicJsonDocument doc(1024);
        doc["type"] = type;
        doc["data"] = data;
        doc["timestamp"] = millis();
        
        String message;
        serializeJson(doc, message);
        client->text(message);
        
        ESP_LOGD("WebSocketManager", "Sent message type: %s to client %u", type.c_str(), clientId);
    }
}

void WebSocketManager::sendStatus(const String& status, const JsonDocument& data) {
    DynamicJsonDocument statusData(1024);
    statusData["status"] = status;
    statusData["data"] = data;
    
    broadcast("status", statusData);
}

void WebSocketManager::sendError(const String& error, uint32_t clientId) {
    DynamicJsonDocument errorData(512);
    errorData["error"] = error;
    errorData["timestamp"] = millis();
    
    if (clientId == 0) {
        broadcast("error", errorData);
    } else {
        broadcastToClient(clientId, "error", errorData);
    }
}

size_t WebSocketManager::getClientCount() const {
    return ws->count();
}

void WebSocketManager::cleanupClients(size_t maxClients) {
    ws->cleanupClients(maxClients);
}

bool WebSocketManager::isClientConnected(uint32_t clientId) {
    auto client = ws->client(clientId);
    return client && client->status() == WS_CONNECTED;
}

void WebSocketManager::pingAllClients() {
    ws->pingAll();
    ESP_LOGD("WebSocketManager", "Pinged all %d clients", getClientCount());
}