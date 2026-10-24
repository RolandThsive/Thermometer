#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "AS5600.h"


// Motor Parameters Structure
struct MotorParam {
    uint8_t MotorNumber;
    uint16_t M_ZERO;
    int8_t EncoderDir;
    int8_t MotorDir;
    uint8_t PCAChannel;
    uint8_t Motor_Pin1;
    uint8_t Motor_Pin2;
    uint8_t MotorChannel_Pin1;
    uint8_t MotorChannel_Pin2;

};

extern MotorParam M1,M2;
#define SLEEP_PIN D8

#define SHT40_PCA_CHANNEL  2
#define M1_AS5600_CHANNEL  1
#define M2_AS5600_CHANNEL  3

// ---------- LEDC channel assignment (old API needs explicit channels) ----------
const int CH_AIN1 = 0;
const int CH_AIN2 = 1;
const int CH_BIN1 = 2;
const int CH_BIN2 = 3;

// ---------- PWM configuration ----------
const int PWM_FREQ_HZ = 40000;     // 25 kHz -> above audible range
const int PWM_RESOLUTION_BITS = 8; // duty range 0-255
const int MIN_PWM = 160;
const int MAX_PWM = 180; // 255
const int PWM_CENTER_DUTY = (MIN_PWM + MAX_PWM)/2;

// Operational Constants, PID Gains and other parameters
#define DEADBAND  5
#define PID_SAFETY_TIMEOUT 10000 // In milliSeconds
#define PID_CYCLES 10

// If the encoder can't be read this many times in a row during a
// positioning move, the move is aborted and the motor stopped rather than
// continuing to drive on a stale/invalid input.
#define MAX_CONSECUTIVE_SENSOR_FAULTS 15

const double Kp = 0.5;
const double Ki = 1.0;
const double Kd = 0.2;

// Total valid travel span fixed at 300 degrees out of 360 degrees
// 360 degrees = 4096 counts -> 300 degrees = 3413 counts
const int TOTAL_SPAN_COUNTS = 3414;
const int FORBIDDEN_ZONE_SIZE = 683; // 4096 - 3413 counts (60 degrees)

// Core Hardware Controls
void motorInitialize();
void motorSleep();
void motorWake();
void motorStop(const MotorParam &Motor);
void motorDrive(const MotorParam &Motor, double controlOutput, double currentError);

// Algorithm Helpers
double as5600CalculateShortestError(double target, double actual);
int virtualAngleCalc(int rawAngle, const MotorParam &Motor);

// Positioning Execution Routines
void runPositioningLoop(const MotorParam &Motor, float targetPos);
void moveProngToPosition(const MotorParam &Motor, float targetPos);


#endif // MOTOR_H
