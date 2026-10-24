/*
  position_pid.h
  ------------------------------------------------
  DRV8833 + 2x N20 motors + 2x AS5600 encoders (via PCA9546A I2C mux)
  on Seeed XIAO ESP32-C3. Closed-loop SHAFT POSITION control per motor.

  Wiring:
    DRV8833 AIN1  -> D0      (Motor A)
    DRV8833 AIN2  -> D1      (Motor A)
    DRV8833 BIN1  -> D2      (Motor B)
    DRV8833 BIN2  -> D3      (Motor B)
    DRV8833 nSLEEP -> D8     (shared, both motors)
    I2C SDA -> D4, SCL -> D5 (shared bus: PCA9546A + both AS5600s)
    PCA9546A: AS5600 for Motor A on mux channel 1
              AS5600 for Motor B on mux channel 3
              (change MUX_CHANNEL_A / MUX_CHANNEL_B below if different)

  PCA9546A address: assumed 0x70 (A0=A1=GND, the default). If your board
  straps A0/A1 differently, the address can be 0x70-0x73 - check your
  board silkscreen/datasheet and update PCA9546A_ADDR.

  Each AS5600 is a single-turn 12-bit absolute encoder at the FIXED
  address 0x36 - since both encoders share that address, the mux
  channel select is what distinguishes them. Only one mux channel can
  be active at a time, so every encoder read first tells the mux which
  channel to route, then talks to 0x36.

  Motor drive uses LOCKED-ANTIPHASE PWM (both pins always switching)
  for quieter operation than sign-magnitude drive, plus slew-rate
  limiting and a deadband to avoid dither/buzz while holding position.
*/

#ifndef POSITION_H
#define POSITION_H

#include <Arduino.h>
#include <Wire.h>

// ---------- Shared bus / hardware ----------
extern const int NSLEEP;
extern const int I2C_SDA;
extern const int I2C_SCL;
extern const uint8_t PCA9546A_ADDR;
extern const uint8_t AS5600_ADDR;
extern const float COUNTS_PER_DEGREE; // 4096/360

// ---------- PWM configuration (shared) ----------
extern const int PWM_FREQ_HZ;
extern const int PWM_RESOLUTION_BITS;
extern const int PWM_MAX_DUTY;
extern const int PWM_CENTER_DUTY;

// ---------- Per-axis mux channels ----------
extern const uint8_t MUX_CHANNEL_A;
extern const uint8_t MUX_CHANNEL_B;

// ---------- One independently-controlled motor + encoder ----------
struct PositionAxis {
  // Hardware
  int pinIN1, pinIN2;      // DRV8833 input pins
  int chIN1, chIN2;        // LEDC PWM channels
  uint8_t muxChannel;      // PCA9546A channel this axis's AS5600 is on

  // PID gains (tune per motor/load)
  float kp, ki, kd;

  // Motion-quality limits
  float outputClamp;
  float slewLimitPerS;
  float deadbandDegrees;

  // Setpoint
  float targetDegrees;

  // Internal state - do not set directly, initialized by axisInit()
  int lastRawAngle;
  long turnCountCounts;
  float pidIntegral;
  float pidPrevMeasurement;
  bool pidFirstRun;
  float lastOutput;
};

// ---------- Global setup ----------
// Call once from setup(). Brings up I2C, DRV8833 sleep pin, and PWM for
// both axes. Does NOT touch axis.targetDegrees or PID gains - set those
// yourself (or take the defaults given in the .cpp) before calling.
void positionControlInit(PositionAxis &axisA, PositionAxis &axisB);

// ---------- Per-axis functions ----------

// Selects this axis's mux channel, then reads its AS5600 raw angle
// (0-4095). Returns -1 on I2C error.
int axisReadAS5600Raw(PositionAxis &axis);

// Updates and returns this axis's continuous (unwrapped) position in
// degrees. Call once per control loop iteration per axis.
float axisUpdatePosition(PositionAxis &axis);

// One PID iteration for this axis. Returns signed motor command.
float axisPidUpdate(PositionAxis &axis, float currentDegrees, float dtSeconds);

// Drives this axis's motor with a signed command using locked-antiphase PWM.
void axisDriveMotor(PositionAxis &axis, float signedOutput);

#endif // POSITION_PID_H