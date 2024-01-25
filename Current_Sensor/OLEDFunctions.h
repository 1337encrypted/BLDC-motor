#pragma once
#include <U8g2lib.h>

namespace OLEDFunctions {
    extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;

    void initOLED();
    void displayCurrent(const char* current);
    void displayRPM(const char* rpm);

} // namespace OLEDFunctions



// // OLEDFunctions.cpp
// #include "OLEDFunctions.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0);

void OLEDFunctions::initOLED() {
    oled.setColorIndex(1);
    oled.begin();
    oled.setBitmapMode(1);
    oled.setFont(u8g2_font_Pixellari_tf);
}

void OLEDFunctions::displayCurrent(const char* current) {
    oled.clearBuffer();
    oled.drawStr(10, 5 + 15 + 2, "Current: ");
    oled.drawStr(70, 5 + 15 + 2, current);
    oled.sendBuffer();
}

void OLEDFunctions::displayRPM(const char* rpm) {
    oled.clearBuffer();
    oled.drawStr(40, 35 + 15 + 2, rpm);
    oled.sendBuffer();
}


