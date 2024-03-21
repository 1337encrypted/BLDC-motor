#pragma once

#include <U8g2lib.h>

namespace OLEDFunctions {
  // Declare extern instance of U8G2_SH1106_128X64_NONAME_F_HW_I2C oled
  extern U8G2_SH1106_128X64_NONAME_F_HW_I2C oled;

  // Define character arrays for OLED display
  char oledSpeed1[6]; // Assuming the maximum size of your speed won't exceed 5 digits + null terminator
  char oledSpeed2[6]; // Assuming the maximum size of your speed won't exceed 5 digits + null terminator
  char currentArr1[6];
  char currentArr2[6];
  char voltageArr1[6];
  
  // Declare functions
  void begin(const BaseType_t);
  void displayRPM();
  void printUartData();
  void renderScreen();
  void clearBuffer();
  void sendBuffer();

  // Declare the renderScreenTask function
  void renderScreenTask(void *);
} // namespace OLEDFunctions

// Define U8G2_SH1106_128X64_NONAME_F_HW_I2C oled instance
U8G2_SH1106_128X64_NONAME_F_HW_I2C OLEDFunctions::oled(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// Define the functions within the namespace
void OLEDFunctions::begin(const BaseType_t app_cpu) {
  oled.setColorIndex(1);
  oled.begin();
  oled.setBitmapMode(1);
  oled.setFont(u8g2_font_Pixellari_tf);
  clearBuffer();

  // Clear character arrays
  memset(oledSpeed1, 0, sizeof(oledSpeed1));
  memset(oledSpeed2, 0, sizeof(oledSpeed2));
  memset(currentArr1, 0, sizeof(currentArr1));
  memset(currentArr2, 0, sizeof(currentArr2));
  memset(voltageArr1, 0, sizeof(voltageArr1));

  // Create renderScreenTask
  xTaskCreatePinnedToCore(&renderScreenTask, "renderScreenTask", 2048, NULL, 1, NULL, app_cpu);
}

// Define utility functions 
void OLEDFunctions::clearBuffer() {
  oled.clearBuffer();
}

void OLEDFunctions::sendBuffer() {
  oled.sendBuffer();
}

void OLEDFunctions::displayRPM() {
  // Convert speed to string and store it in tempSpeed
  oled.drawStr(5, 12, "Speed1: ");   //1
  oled.drawStr(74, 12, oledSpeed1);  //1

  oled.drawStr(5, 25, "Speed2: ");  //1+11+2
  oled.drawStr(74, 25, oledSpeed2);  //1+11+2
}

void OLEDFunctions::printUartData() {

  oled.drawStr(5, 38, "Current1: ");   //1+11+2+11+2
  oled.drawStr(74, 38, currentArr1);   

  oled.drawStr(5, 51, "Current2: ");   //1+11+2+11+2
  oled.drawStr(74, 51, currentArr2);  

  oled.drawStr(5, 64, "Volts: ");   //1+11+2+11+2
  oled.drawStr(74, 64, voltageArr1);  
}

void OLEDFunctions::renderScreen() {
  oled.clearBuffer();
  displayRPM();
  printUartData();
  sendBuffer();
}

void OLEDFunctions::renderScreenTask(void *pvParameters) {
  // This task doesn't need an instance parameter
  while(true) {
    renderScreen();
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}
