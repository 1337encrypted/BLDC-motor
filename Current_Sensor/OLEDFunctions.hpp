#pragma once

#include <U8g2lib.h>
#include <SPI.h>

namespace OLEDFunctions {
    // extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;
     extern U8G2_SH1106_128X64_NONAME_F_HW_I2C oled;
    

    void begin();
    void displayCurrent(const char*);
    void displayVoltage(const char*);
    void displayPower(const char*);

} // namespace OLEDFunctions


// U8G2_SSD1306_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0);
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

void OLEDFunctions::begin(){
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

void OLEDFunctions::displayVoltage(const char* voltage) {
    oled.clearBuffer();
    oled.drawStr(40, 35 + 15 + 2, "Voltage");
    oled.drawStr(40, 35 + 15 + 2, voltage);
    oled.sendBuffer();
}

void OLEDFunctions::displayPower(const char* power) {
    oled.clearBuffer();
    oled.drawStr(40, 35 + 15 + 2, "Power");
    oled.drawStr(40, 35 + 15 + 2, power);
    oled.sendBuffer();
}



