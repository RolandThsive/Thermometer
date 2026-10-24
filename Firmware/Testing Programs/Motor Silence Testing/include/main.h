/*
  drv8833_silent_pwm.h
  ------------------------------------------------
  DRV8833 + N20 motors on Seeed Studio ESP32-C3
  Legacy ESP32 Arduino core 2.x LEDC API (channel-based).

  Drives the motors with a PWM frequency ABOVE human hearing (25 kHz)
  so you don't get the audible coil whine you'd hear at typical
  motor PWM frequencies (~1 kHz).

  Wiring (change pin numbers in the .cpp to match your wiring):
    DRV8833 AIN1 -> GPIO2   (Motor A, pin 1)
    DRV8833 AIN2 -> GPIO3   (Motor A, pin 2)
    DRV8833 BIN1 -> GPIO4   (Motor B, pin 1)
    DRV8833 BIN2 -> GPIO5   (Motor B, pin 2)
    DRV8833 nSLEEP -> GPIO6 (must be HIGH to enable the driver)
    DRV8833 VM -> motor supply voltage (NOT 3.3V/5V from the board
                  unless that's actually your motor voltage)
    DRV8833 GND -> ESP32-C3 GND (common ground)
*/

#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

// ---------- User-configurable pins ----------
extern const int AIN1;
extern const int AIN2;
extern const int BIN1;
extern const int BIN2;
extern const int NSLEEP;

// ---------- LEDC channel assignment (old API needs explicit channels) ----------
extern const int CH_AIN1;
extern const int CH_AIN2;
extern const int CH_BIN1;
extern const int CH_BIN2;

// ---------- PWM configuration ----------
extern const int PWM_FREQ_HZ;        // 25 kHz -> above audible range
extern const int PWM_RESOLUTION_BITS; // duty range 0-255
extern const int PWM_MAX_DUTY;        // 255

// ---------- Public functions ----------

// Call once from setup(). Configures nSLEEP pin and all 4 PWM channels.
void motorsInit();

// speed: 0-255, forward: true = forward, false = reverse
void driveMotor(int chIN1, int chIN2, int speed, bool forward);
void driveMotorAntiphase(int chIN1, int chIN2, int speed, bool forward);

// Coast stop (both pins low). For active braking, drive both pins HIGH instead.
void stopMotor(int chIN1, int chIN2);

#endif // DRV8833_SILENT_PWM_H