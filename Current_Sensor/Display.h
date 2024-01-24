#include <U8g2lib.h>

// U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C oled(U8G2_R0, /* clock=*/ A5, /* data=*/ A4, /* reset=*/ U8X8_PIN_NONE);

void oledBegin()
{
  oled.setColorIndex(1);        // set the color to white
  oled.begin();                 // display in the screen
  oled.setBitmapMode(1);        // able to merge two images, draws only white pixels
  oled.setFont(u8g2_font_Pixellari_tf);
}

void writeCurrent(const char* message)
{
  oled.clearBuffer();  // clear buffer for storing display content in RAM
  oled.drawStr(10,5+15+2, "Current: ");
  oled.drawStr(70,5+15+2, message); 
  oled.sendBuffer(); // send buffer from RAM to display controller 
}

void writeRPM(const char* message)
{
  oled.clearBuffer();  // clear buffer for storing display content in RAM
  oled.drawStr(40,35+15+2, message); 
  oled.sendBuffer(); // send buffer from RAM to display controller 
}