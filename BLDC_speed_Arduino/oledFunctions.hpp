#pragma once

#include <U8g2lib.h>

namespace OLEDFunctions {
  extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled;
  // extern U8G2_SH1106_128X64_NONAME_F_HW_I2C oled;
  

  void begin();
  void test();
  void displayRPM(const char*, uint8_t);
  void clearBuffer();
  void sendBuffer();

} // namespace OLEDFunctions


// U8G2_SSD1306_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
// U8G2_SH1106_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

void OLEDFunctions::begin(){
  oled.setColorIndex(1);
  oled.begin();
  oled.setBitmapMode(1);
  oled.setFont(u8g2_font_Pixellari_tf);
  clearBuffer();
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

void OLEDFunctions::displayRPM(const char* rpm, uint8_t motorId) {

  // oled.clearBuffer();

  // if(motorId == 1){
  //   oled.drawStr(5, 12, "motor1: ");   //1
  //   oled.drawStr(74, 12, rpm);      //1
  // }
  // else if(motorId == 2){
  //   oled.drawStr(5, 25, "motor2: ");   //1+11+2
  //   oled.drawStr(74, 25, rpm);      //1+11+2
  // }
// 
  // oled.sendBuffer();
  // }
  // ESP_LOGI("MOTOR", "rpm %u", rpm);
  Serial.print("\t");
  Serial.print(motorId);
  Serial.print(": ");
  Serial.println(rpm);
}


