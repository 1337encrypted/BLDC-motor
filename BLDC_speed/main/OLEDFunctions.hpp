#pragma once

#include <U8g2lib.h>

namespace OLEDFunctions {
    // extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;
     extern U8G2_SH1106_128X64_NONAME_F_HW_I2C oled;
    

    void begin();
    void displayCurrent(const char*, const char*);
    void displayVoltage(const char*);
    void displayPower(const char*, const char*);
    void printOLED();

    void clearBuffer();
    void sendBuffer();

} // namespace OLEDFunctions


// U8G2_SSD1306_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0);
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

void OLEDFunctions::begin(){
    oled.setColorIndex(1);
    oled.begin();
    oled.setBitmapMode(1);
    oled.setFont(u8g2_font_Pixellari_tf);
    clearBuffer();
}

// Utility functions 
void OLEDFunctions::clearBuffer(){
  oled.clearBuffer();
}

void OLEDFunctions::sendBuffer(){
  oled.sendBuffer();
}

void OLEDFunctions::displayCurrent(const char* current1, const char* current2) {
  oled.drawStr(5, 12, "Current1: ");   //1
  oled.drawStr(74, 12, current1);      //1

  oled.drawStr(5, 25, "Current2: ");   //1+11+2
  oled.drawStr(74, 25, current2);      //1+11+2
}

void OLEDFunctions::displayVoltage(const char* voltage1) {
  oled.drawStr(5, 38, "Voltage1: ");     //1+11+2+11+2
  oled.drawStr(74, 38, voltage1);        //1+11+2+11+2
}

void OLEDFunctions::displayPower(const char* power1, const char* power2) {
    oled.drawStr(7, 51, "Power1: ");      //1+11+2+11+2+11+2
    oled.drawStr(74, 51, power1);

    oled.drawStr(7, 64, "Power2: ");      //1+11+2+11+2+11+2+11+2
    oled.drawStr(74, 64, power2);
}



