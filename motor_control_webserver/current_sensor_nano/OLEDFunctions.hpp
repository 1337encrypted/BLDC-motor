#pragma once

#include <U8g2lib.h>
#include <SPI.h>

namespace OLEDFunctions {
    extern U8G2_SH1106_128X64_NONAME_1_HW_I2C oled;

    void begin();
    void displayCurrent(const char*, const char*);
    void displayVoltage(const char*);
    void displayPower(const char*, const char*);
    void displayIPAddress(const char*);
    void displayWiFiMode(char mode, const char* ipAddress);
    void displayEnhanced(const char* current1, const char* current2, const char* voltage, 
                        const char* power1, const char* power2, const char* ipAddress, 
                        char wifiMode, int wifiStrength);
    
    void clearBuffer();
    void sendBuffer();

} // namespace OLEDFunctions


// Use page buffer mode instead of full frame buffer to save RAM
U8G2_SH1106_128X64_NONAME_1_HW_I2C OLEDFunctions::oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

void OLEDFunctions::begin(){
    oled.setColorIndex(1);
    oled.begin();
    oled.setBitmapMode(1);
    oled.setFont(u8g2_font_7x14_tf); // Bigger font for better readability
    clearBuffer();
}

// Utility functions for page buffer mode
void OLEDFunctions::clearBuffer(){
  oled.firstPage();
}

void OLEDFunctions::sendBuffer(){
  // In page buffer mode, this is handled by the do-while loop
  // This function is kept for compatibility but does nothing
}

void OLEDFunctions::displayCurrent(const char* current1, const char* current2) {
  oled.drawStr(2, 20, "I1:");
  oled.drawStr(25, 20, current1);
  oled.drawStr(60, 20, "A");

  oled.drawStr(2, 35, "I2:");
  oled.drawStr(25, 35, current2);
  oled.drawStr(60, 35, "A");
}

void OLEDFunctions::displayVoltage(const char* voltage1) {
  oled.drawStr(2, 50, "V:");
  oled.drawStr(25, 50, voltage1);
  oled.drawStr(60, 50, "V");
}

void OLEDFunctions::displayPower(const char* power1, const char* power2) {
    oled.drawStr(2, 44, "P1:");
    oled.drawStr(18, 44, power1);
    oled.drawStr(45, 44, "W");

    oled.drawStr(65, 44, "P2:");
    oled.drawStr(81, 44, power2);
    oled.drawStr(108, 44, "W");
}

void OLEDFunctions::displayIPAddress(const char* ipAddress) {
    // Display IP address at the bottom of the screen with bigger font
    oled.setFont(u8g2_font_6x10_tf); // Medium font for IP
    oled.drawStr(2, 62, ipAddress);
    oled.setFont(u8g2_font_7x14_tf); // Reset to main font
}

void OLEDFunctions::displayWiFiMode(char mode, const char* ipAddress) {
    // Display WiFi mode and IP address at the bottom
    oled.setFont(u8g2_font_4x6_tf); // Smaller font for mode info
    
    // First line: Mode indicator
    if (mode == 'A') {
        oled.drawStr(2, 58, "FIELD MODE (AP)"); // AP Mode (Field operation)
    } else if (mode == 'S') {
        oled.drawStr(2, 58, "HOME MODE (STA)"); // Station Mode (Home/Office)
    } else {
        oled.drawStr(2, 58, "WIFI MODE");
    }
    
    // Second line: IP address
    oled.drawStr(2, 64, ipAddress);
    oled.setFont(u8g2_font_6x10_tf); // Reset to main font
}

void OLEDFunctions::displayEnhanced(const char* current1, const char* current2, const char* voltage, 
                                   const char* power1, const char* power2, const char* ipAddress, 
                                   char wifiMode, int wifiStrength) {
    // Simplified display with bigger fonts - current, voltage, and IP only
    oled.setFont(u8g2_font_7x14_tf); // Bigger font for main data
    
    // Main content area - current and voltage with larger spacing
    displayCurrent(current1, current2);
    displayVoltage(voltage);
    
    // Bottom section - IP address (always show if available)
    if (ipAddress && strlen(ipAddress) > 0 && strcmp(ipAddress, "No IP Set") != 0) {
        displayIPAddress(ipAddress);
    }
}