#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "main.h"
#include "encoder.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &PendulumI2C, OLED_RESET);

unsigned long lastOledTime = 0;
const unsigned long oledRefreshInterval = 150;

void setup_oled(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED allocation failed! Check your wiring."));
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 20);
    display.println(" SYSTEM INIT... ");
    display.display();
  }
  lastOledTime = millis();
}

void loop_oled(){
  if (millis() - lastOledTime >= oledRefreshInterval) {
    lastOledTime = millis();
    
    display.clearDisplay();
    display.setTextSize(1);
    //display.setCursor(0, 0);
    //display.print("INICIANDO...");
    display.setTextSize(2);
    display.setCursor(0, 0);
    //display.print("Cart Pos: ");
    display.print(gCartPosition, 1);
    display.setCursor(0, 21);
    //display.print("Th: ");
    display.print(gPend, 2);
    display.setCursor(0, 42);
    //display.print("ThS: ");
    display.print(gPendSpeed, 2);
    /*
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("PENDULUM ANGLE:");
    display.display();
    */
  }
}
