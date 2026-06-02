#include <Arduino.h>
#include <Wire.h>

#define AS5600_ADDR 0x36
#define RAW_ANGLE_H 0x0C

TwoWire PendulumI2C = TwoWire(0);
TwoWire CartI2C     = TwoWire(1);

#define PEND_SDA 16
#define PEND_SCL 17
#define CART_SDA 21
#define CART_SCL 22
#define IN1 18
#define IN2 19

#define PWM_FREQ 1000
#define PWM_RESOLUTION 8

const float pendOffsetDeg = -145.55 + 180; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// timing
unsigned long lastTime = 0;

// zero references
int cartZero = 0;
int pendZero = 0;

// previous values
float lastCart = 0;
float lastPend = 0;

// outputs
float x = 0, x_dot = 0;
float theta = 0, theta_dot = 0;

void setup() {
  Serial.begin(115200);

  PendulumI2C.begin(PEND_SDA, PEND_SCL, 100000);
  CartI2C.begin(CART_SDA, CART_SCL, 100000);
  ledcAttach(IN1, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(IN2, PWM_FREQ, PWM_RESOLUTION);

  ledcWrite(IN1, 0);
  ledcWrite(IN2, 0);

  cartZero = readRaw(CartI2C);
  pendZero = readRaw(PendulumI2C);
  
  delay(1000);
  lastTime = millis();
}

void loop() {
  unsigned long now = millis();
  float dt = (now - lastTime) / 1000.0;
  lastTime = now;

  if (dt <= 0) return;

  // raw readings
  int rawCart = readRaw(CartI2C);
  int rawPend = readRaw(PendulumI2C);

  // unwrap relative to zero
  int dCart = rawCart - cartZero;
  int dPend = rawPend - pendZero;

  if (dCart > 2048) dCart -= 4096;
  if (dCart < -2048) dCart += 4096;

  if (dPend > 2048) dPend -= 4096;
  if (dPend < -2048) dPend += 4096;

  // convert to degrees
  float cart = dCart * 360.0 / 4096.0;
  float pend = dPend * 360.0 / 4096.0;
  
  // shift so upright becomes zero
  pend = pend - pendOffsetDeg;
  if (pend > 180) pend -= 360;
  if (pend < -180) pend += 360;
  // velocity (no filtering)
  x_dot = (cart - lastCart) / dt;
  theta_dot = (pend - lastPend) / dt;

  lastCart = cart;
  lastPend = pend;

  x = cart;
  theta = pend;

  // debug
  Serial.print("x: ");
  Serial.print(x, 2);

  Serial.print(" | x_dot: ");
  Serial.print(x_dot, 2);

  Serial.print(" | theta: ");
  Serial.print(theta, 2);

  Serial.print(" | theta_dot: ");
  Serial.println(theta_dot, 2);

  float Kp = 8.0;
  float Kd = 0.5;

  float u = 0;
  if (abs(theta) < 45) {
    u = Kp * theta + Kd * theta_dot; // PD
  } else {
    u = 0;
  }

  u = constrain(u, -255, 255);

  int pwm = abs(u);

  // not enough strength on less than 78
  if (pwm < 78) pwm = 0;
  if (pwm > 0)
      pwm += 130;

  pwm = constrain(pwm, 130, 255);

  if (u > 3) {
    ledcWrite(IN1, pwm);
    ledcWrite(IN2, 0);
  } else if (u < -3) {
    ledcWrite(IN1, 0);
    ledcWrite(IN2, pwm);
  } else {
    ledcWrite(IN1, 0);
    ledcWrite(IN2, 0);
  }

  Serial.print(" | u: ");
  Serial.println(u);
  delay(10);
}

int readRaw(TwoWire &bus) {
  bus.beginTransmission(AS5600_ADDR);
  bus.write(RAW_ANGLE_H);

  if (bus.endTransmission(false) != 0) return 0;
  if (bus.requestFrom((uint8_t)AS5600_ADDR, (uint8_t)2) != 2) return 0;

  uint8_t high = bus.read();
  uint8_t low  = bus.read();

  return ((high << 8) | low) & 0x0FFF;
}
