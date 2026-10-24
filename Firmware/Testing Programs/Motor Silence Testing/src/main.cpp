
#include <Arduino.h>
#include <Wire.h>
#include "main.h"

// ---------- User-configurable pins ----------
const int AIN1 = D0;
const int AIN2 = D1;
const int BIN1 = D2;
const int BIN2 = D3;
const int NSLEEP = D8;

// ---------- LEDC channel assignment (old API needs explicit channels) ----------
const int CH_AIN1 = 0;
const int CH_AIN2 = 1;
const int CH_BIN1 = 2;
const int CH_BIN2 = 3;
 
// ---------- PWM configuration ----------
const int PWM_FREQ_HZ = 25000;     // 25 kHz -> above audible range
const int PWM_RESOLUTION_BITS = 8; // duty range 0-255
const int PWM_MAX_DUTY = (1 << PWM_RESOLUTION_BITS) - 1; // 255
 
void setup() {
  Serial.begin(115200);
 
  pinMode(NSLEEP, OUTPUT);
  digitalWrite(NSLEEP, HIGH); // wake the driver up
 
  // Configure each PWM channel at 25kHz / 8-bit resolution
  ledcSetup(CH_AIN1, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
  ledcSetup(CH_AIN2, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
  ledcSetup(CH_BIN1, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
  ledcSetup(CH_BIN2, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
 
  // Attach each channel to its physical pin
  ledcAttachPin(AIN1, CH_AIN1);
  ledcAttachPin(AIN2, CH_AIN2);
  ledcAttachPin(BIN1, CH_BIN1);
  ledcAttachPin(BIN2, CH_BIN2);
 
  stopMotor(CH_AIN1, CH_AIN2);
  stopMotor(CH_BIN1, CH_BIN2);
}
 
void loop() {
  // --- Demo sequence, replace with your own logic ---


 
  // Ramp motor A speed up and down, silently
  for (int duty = 150; duty <= 180; duty += 1) {
    driveMotorAntiphase(CH_AIN1, CH_AIN2, duty, true);
    driveMotorAntiphase(CH_BIN1, CH_BIN2, duty, true);

    delay(15);
  }
  for (int duty = 180; duty >= 150; duty -= 1) {
    driveMotorAntiphase(CH_AIN1, CH_AIN2, duty, true);
    driveMotorAntiphase(CH_BIN1, CH_BIN2, duty, true);
    delay(15);
  }
 
  stopMotor(CH_AIN1, CH_AIN1);
  stopMotor(CH_BIN1, CH_BIN2);
  delay(3000);
}

 
// ---------- Motor helper functions ----------
 
// speed: 0-255, forward: true = forward, false = reverse
void driveMotor(int chIN1, int chIN2, int speed, bool forward) {
  speed = constrain(speed, 0, PWM_MAX_DUTY);
  if (forward) {
    ledcWrite(chIN1, speed);
    ledcWrite(chIN2, 0);
  } else {
    ledcWrite(chIN1, 0);
    ledcWrite(chIN2, speed);
  }
}

void driveMotorAntiphase(int chIN1, int chIN2, int speed, bool forward) {
  // speed: -255 to 255 isn't used here; instead map duty around center
  int duty = map(speed, 0, 255, 128, 255); // 128 = stopped, 255 = full fwd
  if (!forward) duty = 256 - duty;
  ledcWrite(chIN1, duty);
  ledcWrite(chIN2, PWM_MAX_DUTY - duty); // inverted
}
 
// Coast stop (both pins low). For active braking, drive both pins HIGH instead.
void stopMotor(int chIN1, int chIN2) {
  ledcWrite(chIN1, 0);
  ledcWrite(chIN2, 0);
}