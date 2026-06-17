#include <Arduino.h>
#include <Wire.h>

#include "encoder.h"
#include "oled.h"

//TwoWire PendulumI2C = TwoWire(0);
//TwoWire CartI2C     = TwoWire(1);

// Pins
#define PEND_SDA 2
#define PEND_SCL 3
#define CART_SDA 8
#define CART_SCL 9

#define IN1 6
#define IN2 7

#define PWM_FREQ 1000
#define PWM_RESOLUTION 8 

const float pendOffsetDeg = 69.08 + 110.57; 

unsigned long gLastTime = 0;
float gPend = 0;
float gPendSpeed = 0;

int cartZero = 0;
int pendZero = 0;

float lastCart = 0;
float lastPend = 0;

// Kinematic outputs
float x = 0, x_dot = 0;
float theta = 0, theta_dot = 0, last_theta_dot = 0;

#define AS5600_ADDR 0x36
#define RAW_ANGLE_H 0x0C

//int readRaw(TwoWire &bus);

int readRaw() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(RAW_ANGLE_H);

  if (Wire.endTransmission(false) != 0) return 0;
  if (Wire.requestFrom((uint8_t)AS5600_ADDR, (uint8_t)2) != 2) return 0;

  uint8_t high = Wire.read();
  uint8_t low  = Wire.read();

  return ((high << 8) | low) & 0x0FFF; 
}

int approach(int current, int target, int step, unsigned long intervalMs, unsigned long &lastStepTime) {
    unsigned long now = millis();

    if (now - lastStepTime < intervalMs)
        return current;

    lastStepTime = now;

    if (current < target)
        return min(current + step, target);

    if (current > target)
        return max(current - step, target);

    return current;
}

void setup() {
  setup_encoder();
  Wire.begin(PEND_SDA, PEND_SCL, 400000); // 400kHz fast mode
  setup_oled();

  ledcSetup(0, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(1, PWM_FREQ, PWM_RESOLUTION);

  ledcAttachPin(IN1, 0);
  ledcAttachPin(IN2, 1);

  ledcWrite(0, 0);
  ledcWrite(1, 0);

  pendZero = readRaw();
  delay(500);
  gLastTime = millis();
  

  delay(3000);
}

enum modes {
  MODE_CENTER,
  MODE_SWINGUP,
  MODE_BALANCE
};

bool centering_done = false;
int gRawPend = 0;
int gPendulumMode = MODE_CENTER;

void loop() { 
  gRawPend = readRaw();
  unsigned long now = millis();
  float dt = (now - gLastTime) / 1000.0;
  gLastTime = now;

  if (dt <= 0) return;

  int dPend = gRawPend - pendZero;

  if (dPend > 2048)  dPend -= 4096;
  if (dPend < -2048) dPend += 4096;

  // steps to angular degrees
  gPend = dPend * 360.0 / 4096.0;
  
  // shift so pendulum being upright becomes zero
  gPend = gPend - pendOffsetDeg;
  if (gPend > 180)  gPend -= 360;
  if (gPend < -180) gPend += 360;

  // dt
  gPendSpeed = theta_dot = (gPend - lastPend) / dt;

  //lastCart = cart;
  lastPend = gPend;

  //x = cart;
  theta = gPend;

  // PD Control
  float Kp = 8.0;
  float Kd = 0.5;
  float u = 0;
  
  if (abs(theta) < 45) {
    u = (Kp * theta) + (Kd * theta_dot); 
  } else {
    u = 0;
  }

  u = constrain(u, -255, 255);
  int pwm = abs(u);

  // Motor won't move cart below roughly 78, so disable it there
  if (pwm < 78) pwm = 0;
  if (pwm > 0)  pwm += 130;
  pwm = constrain(pwm, 130, 255);

  // DRV8871
  if ((gCartPosition < 25) && (gCartPosition > 4.8) || (gPendulumMode == MODE_CENTER)) {
    switch(gPendulumMode) {
      case MODE_CENTER:
        if((gCartPosition < 13.2)){
          ledcWrite(0, 0);
          ledcWrite(1, 150);
        } else {
          gPendulumMode = MODE_SWINGUP;
          ledcWrite(0, 0);
          ledcWrite(1, 0);
        }
      break;
      case MODE_SWINGUP:
          static int swing_pwm = 0;
          static unsigned long swing_pwm_timer = 0;
          
          if (abs(theta) < 160)  {
            if (last_theta_dot > 0 && theta_dot <= 0) {
                swing_pwm = -150;
            } else if (last_theta_dot < 0 && theta_dot >= 0) {
                swing_pwm = 150;
            }
          }
        swing_pwm = approach(swing_pwm, 0, 1, 20, swing_pwm_timer);
        if (swing_pwm > 0) {
            ledcWrite(0, 0);
            ledcWrite(1, abs(swing_pwm));
          } else if (swing_pwm < 0) {
            ledcWrite(0, abs(swing_pwm));
            ledcWrite(1, 0);
          } else {
            ledcWrite(0, 0);
            ledcWrite(1, 0);
          }
        //gPendulumMode = MODE_BALANCE;
      break;
      case MODE_BALANCE:
      
        if (u > 3) {
            ledcWrite(0, 0);
            ledcWrite(1, pwm);
          } else if (u < -3) {
            ledcWrite(0, pwm);
            ledcWrite(1, 0);
          } else {
            ledcWrite(0, 0);
            ledcWrite(1, 0);
          }
        
      break;
    }
  } else {
    ledcWrite(0, 0);
    ledcWrite(1, 0);
  }

  loop_oled();
  loop_encoder();
  last_theta_dot = theta_dot;
}

/*left, right
  if()
    ledcWrite(0, 150);
    ledcWrite(1, 0);
delay(500);
    ledcWrite(0, 0);
    ledcWrite(1, 150);
    delay(10000);
  */