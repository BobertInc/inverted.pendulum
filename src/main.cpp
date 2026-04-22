#include <Arduino.h>

#define AIN1 7
#define AIN2 6
#define PWMA 5
#define STBY 2

#define PULSADOR_R 4
#define PULSADOR_L 3

void setup() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(STBY, OUTPUT);

  pinMode(PULSADOR_R, INPUT);
  pinMode(PULSADOR_L, INPUT);

  digitalWrite(STBY, HIGH);
}

void motorDerecha(int speed) {
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, speed);
}

void motorIzquierda(int speed) {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, speed);
}

void motorStop() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 0);
}

void loop() {
  int horario = digitalRead(PULSADOR_R);
  int antihorario = digitalRead(PULSADOR_L);

  if (horario && !antihorario) {
    motorDerecha(255);
  } 
  else if (antihorario && !horario) {
    motorIzquierda(255);
  } 
  else {
    motorStop();
  }
}