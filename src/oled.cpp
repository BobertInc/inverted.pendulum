#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "main.h"
#include "encoder.h"


// OLED configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1 // Sharing ESP32 reset pin
#define OLED_ADDR   0x3C // Standard 0.96" OLED I2C address

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &PendulumI2C, OLED_RESET);

unsigned long lastOledTime = 0;
const unsigned long oledRefreshInterval = 150; // Update display every 150ms to save loop speed

void setup_oled(){
      // Initialize the OLED screen
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("OLED allocation failed! Check your wiring."));
    // Don't freeze completely, keep going if you just want to check hardware logs
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
    // --- NON-BLOCKING OLED UPDATE SEGMENT ---
  // Only write to the screen if enough time has passed to safeguard loop speed
  if (millis() - lastOledTime >= oledRefreshInterval) {
    lastOledTime = millis();
    
    display.clearDisplay();
    display.setTextSize(1);
    //display.setCursor(0, 0);
    //display.print("INICIANDO...");
    display.setTextSize(2);
    display.setCursor(0, 0);
    //display.print("Cart Pos: ");
    display.print(gCartPosition, 1); // Prints current angle with 1 decimal accuracy
    display.setCursor(0, 21);
    //display.print("Th: ");
    display.print(gPend, 2);
    display.setCursor(0, 42);
    //display.print("ThS: ");
    display.print(gPendSpeed, 2);
    /*
    
    // Header Label
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("PENDULUM ANGLE:");
    
    // Large Dynamic Reading Frame
    display.setTextSize(3);
    display.setCursor(10, 24);
    display.print(theta, 1); // Prints current angle with 1 decimal accuracy
    display.print((char)247); // Adds degree symbol (°)
    
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.print("JAJA XD PE CAUSE BE CAUSE:");
    */

    // Push buffer data to screen hardware
    display.display();
  }
}