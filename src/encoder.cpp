#include <Arduino.h>

float gCartPosition;

const float TRACK_LENGTH_CM = 29.8f;
const long TRACK_COUNTS = 5220;

volatile long encoderCount = 0;

const int ENC_A = 4;
const int ENC_B = 5;

const long WARNING_LEFT  = 500;
const long WARNING_RIGHT = 4720;

const long LIMIT_LEFT    = 200;
const long LIMIT_RIGHT   = 5020;

void IRAM_ATTR encoderA()
{
    if (digitalRead(ENC_A) == digitalRead(ENC_B))
        encoderCount++;
    else
        encoderCount--;
}

void IRAM_ATTR encoderB()
{
    if (digitalRead(ENC_A) != digitalRead(ENC_B))
        encoderCount++;
    else
        encoderCount--;
}

void setup_encoder() {
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(ENC_A), encoderA, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_B), encoderB, CHANGE);
}

void loop_encoder() {
    gCartPosition = encoderCount * TRACK_LENGTH_CM / TRACK_COUNTS;
    static long last = 0;

    delay(20);

    long c = encoderCount;

    Serial.print("Count: ");
    Serial.print(c);

    Serial.print(" Delta: ");
    Serial.println(c - last);

    last = c;
    /*
    if (encoderCount < LIMIT_LEFT || encoderCount > LIMIT_RIGHT) {
        //pixel.setPixelColor(0, pixel.Color(255, 0, 0));
    } else if (encoderCount < WARNING_LEFT || encoderCount > WARNING_RIGHT) {
        //pixel.setPixelColor(0, pixel.Color(255, 130, 0));
    } else {
        //pixel.setPixelColor(0, pixel.Color(0, 0, 0));
    }
    //pixel.show();
    */
}