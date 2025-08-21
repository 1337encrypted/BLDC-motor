#pragma once

#include <U8g2lib.h>
#include <SPI.h>

namespace OLEDFunctions {
    // extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;
     extern U8G2_SH1106_128X64_NONAME_1_HW_I2C oled;
    

    void begin();
    void displayCurrent(const char*, const char*);
    void displayVoltage(const char*);
    void displayPower(const char*, const char*);
    void displayIPAddress(const char*);
    void displayWiFiMode(char mode, const char* ipAddress);
    void printOLED();

    void clearBuffer();
    void sendBuffer();

} // namespace OLEDFunctions


// Use page buffer mode instead of full frame buffer to save RAM
U8G2_SH1106_128X64_NONAME_1_HW_I2C OLEDFunctions::oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

void OLEDFunctions::begin(){
    oled.setColorIndex(1);
    oled.begin();
    oled.setBitmapMode(1);
    oled.setFont(u8g2_font_Pixellari_tf);
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
  oled.drawStr(5, 12, "I1: ");
  oled.drawStr(74, 12, current1);

  oled.drawStr(5, 25, "I2: ");
  oled.drawStr(74, 25, current2);
}

void OLEDFunctions::displayVoltage(const char* voltage1) {
  oled.drawStr(5, 38, "V: ");
  oled.drawStr(74, 38, voltage1);
}

void OLEDFunctions::displayPower(const char* power1, const char* power2) {
    oled.drawStr(7, 51, "P1: ");
    oled.drawStr(74, 51, power1);

    oled.drawStr(7, 64, "P2: ");
    oled.drawStr(74, 64, power2);
}

void OLEDFunctions::displayIPAddress(const char* ipAddress) {
    // Display IP address at the bottom of the screen (memory optimized)
    // First line: "ESP32 IP:"
    oled.drawStr(5, 51, "ESP32:");
    
    // Second line: actual IP address (max 15 chars)
    oled.drawStr(5, 64, ipAddress);
}

void OLEDFunctions::displayWiFiMode(char mode, const char* ipAddress) {
    // Display WiFi mode and IP address at the bottom
    // First line: Mode indicator
    if (mode == 'A') {
        oled.drawStr(5, 51, "Field:"); // AP Mode (Field operation)
    } else if (mode == 'S') {
        oled.drawStr(5, 51, "Home:"); // Station Mode (Home/Office)
    } else {
        oled.drawStr(5, 51, "WiFi:"); // Unknown mode
    }
    
    // Second line: IP address
    oled.drawStr(5, 64, ipAddress);
}



