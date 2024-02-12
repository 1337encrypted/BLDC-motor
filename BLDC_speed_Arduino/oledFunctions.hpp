#pragma once

#include <U8g2lib.h>

namespace OLEDFunctions {
  // extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;
  extern U8G2_SH1106_128X64_NONAME_F_HW_I2C oled;

  char oledSpeed1[6]; // Assuming the maximum size of your speed won't exceed 5 digits + null terminator
  char oledSpeed2[6]; // Assuming the maximum size of your speed won't exceed 5 digits + null terminator
  

  void begin();
  void test();
  void displayRPM(uint16_t, uint8_t);
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


  memset((void *)(oledSpeed1), 0, sizeof(oledSpeed1));
  memset((void *)(oledSpeed2), 0, sizeof(oledSpeed2));
}

void OLEDFunctions::test(){
  oled.clearBuffer();
  oled.drawStr(5, 12, "motor1: ");   //1
  oled.sendBuffer();
}

// Utility functions 
void OLEDFunctions::clearBuffer(){
  oled.clearBuffer();
}

void OLEDFunctions::sendBuffer(){
  oled.sendBuffer();
}

void OLEDFunctions::displayRPM(uint16_t speed , uint8_t motorId) {

  // Convert speed to string and store it in tempSpeed
  

  if(motorId == 1)
    itoa(speed, oledSpeed1, 10); // Using base 10 
  else
    itoa(speed, oledSpeed2, 10); // Using base 10

  oled.clearBuffer();

  oled.drawStr(5, 12, "motor1: ");   //1
  oled.drawStr(74, 12, oledSpeed1);      //1

  oled.drawStr(5, 25, "motor2: ");   //1+11+2
  oled.drawStr(74, 25, oledSpeed2);      //1+11+2

  oled.sendBuffer();

  // ESP_LOGI("MOTOR", "rpm %u", rpm);
  // Serial.print("\t");
  // Serial.print(motorId);
  // Serial.print(": ");
  // Serial.println(rpm);
}


