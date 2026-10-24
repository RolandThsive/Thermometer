/*
  position_pid.cpp
  ------------------------------------------------
  See position_pid.h for wiring and design notes.
*/

#include "position.h"

// ---------- Shared bus / hardware ----------
const int NSLEEP = D8;
const int I2C_SDA = D4;
const int I2C_SCL = D5;
const uint8_t PCA9546A_ADDR = 0x70; // verify against your board's A0/A1 strapping
const uint8_t AS5600_ADDR = 0x36;   // fixed, same on every AS5600
const float COUNTS_PER_DEGREE = 4096.0f / 360.0f;

// ---------- PWM configuration ----------
const int PWM_FREQ_HZ = 40000;
const int PWM_RESOLUTION_BITS = 8;
const int PWM_MAX_DUTY = 180;//(1 << PWM_RESOLUTION_BITS) - 1; // 255
const int PWM_CENTER_DUTY = PWM_MAX_DUTY / 2;

// ---------- Per-axis mux channels ----------
const uint8_t MUX_CHANNEL_A = 1;
const uint8_t MUX_CHANNEL_B = 3;

// ---------- Mux channel select ----------
// Writing a single bit selects exactly that channel and deselects all
// others (the control register is a direct write, not an OR), so this
// is safe to call before every transaction even if another axis's
// channel was active a moment ago.
static bool selectMuxChannel(uint8_t channel) {
  Wire.beginTransmission(PCA9546A_ADDR);
  Wire.write(1 << channel);
  return (Wire.endTransmission() == 0);
}

static void axisInitPWM(PositionAxis &axis) {
  ledcSetup(axis.chIN1, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
  ledcSetup(axis.chIN2, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
  ledcAttachPin(axis.pinIN1, axis.chIN1);
  ledcAttachPin(axis.pinIN2, axis.chIN2);
  ledcWrite(axis.chIN1, PWM_CENTER_DUTY); // start stopped
  ledcWrite(axis.chIN2, PWM_CENTER_DUTY);
}

static void axisResetState(PositionAxis &axis, int primingRaw) {
  axis.lastRawAngle = primingRaw;
  axis.turnCountCounts = primingRaw;
  axis.pidIntegral = 0.0f;
  axis.pidPrevMeasurement = 0.0f;
  axis.pidFirstRun = true;
  axis.lastOutput = 0.0f;
}

static int primeAxisEncoder(PositionAxis &axis) {
  int raw = axisReadAS5600Raw(axis);
  unsigned long start = millis();
  while (raw < 0 && millis() - start < 1000) {
    delay(10);
    raw = axisReadAS5600Raw(axis);
  }
  if (raw < 0) {
    Serial.print("WARNING: AS5600 on mux channel ");
    Serial.print(axis.muxChannel);
    Serial.println(" not responding - check wiring/address.");
    raw = 0;
  }
  return raw;
}

void positionControlInit(PositionAxis &axisA, PositionAxis &axisB) {
  Serial.begin(115200);

  pinMode(NSLEEP, OUTPUT);
  digitalWrite(NSLEEP, HIGH); // wake the DRV8833

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  axisInitPWM(axisA);
  axisInitPWM(axisB);

  axisResetState(axisA, primeAxisEncoder(axisA));
  axisResetState(axisB, primeAxisEncoder(axisB));
}

int axisReadAS5600Raw(PositionAxis &axis) {
  if (!selectMuxChannel(axis.muxChannel)) {
    return -1; // mux itself didn't ACK
  }

  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0C); // RAW ANGLE high byte register
  if (Wire.endTransmission(false) != 0) {
    return -1;
  }
  uint8_t bytesReceived = Wire.requestFrom((uint8_t)AS5600_ADDR, (uint8_t)2);
  if (bytesReceived < 2) {
    return -1;
  }
  int hi = Wire.read();
  int lo = Wire.read();
  return ((hi & 0x0F) << 8) | lo; // 12-bit value, 0-4095
}

float axisUpdatePosition(PositionAxis &axis) {
  int raw = axisReadAS5600Raw(axis);
  if (raw < 0) {
    // I2C read failed this cycle - hold last known position.
    return axis.turnCountCounts / COUNTS_PER_DEGREE;
  }

  int delta = raw - axis.lastRawAngle;
  if (delta > 2048) {
    delta -= 4096;
  } else if (delta < -2048) {
    delta += 4096;
  }

  axis.turnCountCounts += delta;
  axis.lastRawAngle = raw;

  return axis.turnCountCounts / COUNTS_PER_DEGREE;
}

float axisPidUpdate(PositionAxis &axis, float currentDegrees, float dtSeconds) {
  if (dtSeconds <= 0.0f) dtSeconds = 0.001f;

  float error = axis.targetDegrees - currentDegrees;

  if (fabs(error) < axis.deadbandDegrees) {
    axis.pidIntegral = 0.0f;
    axis.pidPrevMeasurement = currentDegrees;
    axis.lastOutput = 0.0f;
    return 0.0f;
  }

  if (axis.pidFirstRun) {
    axis.pidPrevMeasurement = currentDegrees;
    axis.pidFirstRun = false;
  }

  axis.pidIntegral += error * dtSeconds;
  float integralClamp = axis.outputClamp / max(axis.ki, 0.0001f);
  axis.pidIntegral = constrain(axis.pidIntegral, -integralClamp, integralClamp);

  float derivative = -(currentDegrees - axis.pidPrevMeasurement) / dtSeconds;
  axis.pidPrevMeasurement = currentDegrees;

  float output = axis.kp * error + axis.ki * axis.pidIntegral + axis.kd * derivative;
  output = constrain(output, -axis.outputClamp, axis.outputClamp);

  float maxStep = axis.slewLimitPerS * dtSeconds;
  float delta = constrain(output - axis.lastOutput, -maxStep, maxStep);
  output = axis.lastOutput + delta;
  axis.lastOutput = output;

  return output;
}

void axisDriveMotor(PositionAxis &axis, float signedOutput) {
  signedOutput = constrain(signedOutput, -axis.outputClamp, axis.outputClamp);

  int duty1 = PWM_CENTER_DUTY + (int)signedOutput;
  int duty2 = PWM_CENTER_DUTY - (int)signedOutput;

  duty1 = constrain(duty1, 0, PWM_MAX_DUTY);
  duty2 = constrain(duty2, 0, PWM_MAX_DUTY);

  ledcWrite(axis.chIN1, duty1);
  ledcWrite(axis.chIN2, duty2);
}