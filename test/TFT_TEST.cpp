#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include "BitMap.h"
TFT_eSPI myGLCD = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#define DELAY 500

#define TFT_GREY 0x7BEF
#define TFT_W 128
#define TFT_H 128

unsigned long runTime = 0;
void setup()
{
  
  // Setup the LCD
  myGLCD.init();
  myGLCD.setRotation(1);
  myGLCD.fillScreen(TFT_WHITE);
  myGLCD.drawXBitmap(10,5,temp1,40,40,TFT_RED);
  myGLCD.drawXBitmap(80,5,humi,40,40,TFT_BLUE);
  myGLCD.drawXBitmap(15,68,soil,32,32,TFT_BROWN);
  myGLCD.drawXBitmap(80,68,sun,32,32,TFT_ORANGE);
}

void loop()
{
  myGLCD.fillRect(10,48,42,15,TFT_WHITE);
  myGLCD.fillRect(80,48,42,15,TFT_WHITE);
  myGLCD.fillRect(10,105,42,15,TFT_WHITE);
  myGLCD.fillRect(80,105,42,15,TFT_WHITE);
  float temp = random(10.0,70.0);
  int humi1 = random(0,100);
  int soil1 = random(0,100);
  int lux = random(0,40000);
  //Nhiet do
  myGLCD.setTextColor(TFT_RED);
  myGLCD.setTextSize(2);
  myGLCD.drawFloat(temp,1,10,48);
  myGLCD.setCursor(58,48);
  myGLCD.setTextSize(1);
  myGLCD.printf("%cC",248);
  // Do am ko khi
  myGLCD.setTextColor(TFT_BLUE);
  myGLCD.setTextSize(2);
  myGLCD.drawNumber(humi1,80,48); 
  myGLCD.setCursor(110,48);
  myGLCD.print("%");
  // Do am dat
  myGLCD.setTextColor(TFT_BROWN);
  myGLCD.setTextSize(2);
  myGLCD.drawNumber(soil1,12,105); 
  myGLCD.setCursor(40,105);
  myGLCD.print("%");
   //Anh sang
  myGLCD.setTextColor(TFT_ORANGE);
  myGLCD.setTextSize(1);
  myGLCD.drawNumber(lux,82,105);
  myGLCD.setCursor(90,115);
  myGLCD.setTextSize(1);
  myGLCD.print("lux");
  delay(3000);
}