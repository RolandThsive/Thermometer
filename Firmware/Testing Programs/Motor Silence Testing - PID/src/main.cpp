/*
  main.cpp
  ------------------------------------------------
  Entry point (setup/loop) for the dual-axis AS5600 position PID.
  Same logic as position_pid.ino, just as a .cpp for projects that use
  a main.cpp entry point (e.g. PlatformIO's src/main.cpp) instead of
  the Arduino IDE's .ino-per-folder convention.
*/

#include <Arduino.h>
#include "position.h"

// Motor A: AIN1/AIN2, LEDC channels 0/1, AS5600 on mux channel 1
PositionAxis motorA = {
  /* pinIN1, pinIN2 */ D0, D1,
  /* chIN1, chIN2   */ 0, 1,
  /* muxChannel     */ MUX_CHANNEL_B,
  /* kp, ki, kd      */ 0.5f, 0.5f, 0.2f,
  /* outputClamp     */ 100.0f,
  /* slewLimitPerS   */ 400.0f,
  /* deadbandDegrees */ 0.5f,
  /* targetDegrees   */ 90.0f,
  // remaining fields are internal state, initialized by positionControlInit()
  0, 0, 0.0f, 0.0f, true, 0.0f
};

// Motor B: BIN1/BIN2, LEDC channels 2/3, AS5600 on mux channel 3
PositionAxis motorB = {
  /* pinIN1, pinIN2 */ D2, D3,
  /* chIN1, chIN2   */ 2, 3,
  /* muxChannel     */ MUX_CHANNEL_A,
  /* kp, ki, kd      */ 0.5f, 0.5f, 0.2f,
  /* outputClamp     */ 100.0f,
  /* slewLimitPerS   */ 400.0f,
  /* deadbandDegrees */ 0.5f,
  /* targetDegrees   */ 0.0f,
  0, 0, 0.0f, 0.0f, true, 0.0f
};

unsigned long lastLoopMicros = 0;

void setup() {
  positionControlInit(motorA, motorB);
  Serial.println("Dual-axis position PID ready.");
  Serial.println("Type 'A90' or 'B45' + Enter to set a target angle (degrees).");
  lastLoopMicros = micros();
}

void loop() {
  // Optional: set a target via Serial, e.g. "A90" or "B-45"
  if (Serial.available()) {
    char axisChar = Serial.read();
    float value = Serial.parseFloat();
    while (Serial.available()) Serial.read(); // flush rest of line
    if (axisChar == 'A' || axisChar == 'a') {
      motorA.targetDegrees = value;
      Serial.print("Motor A target: "); Serial.println(value);
    } else if (axisChar == 'B' || axisChar == 'b') {
      motorB.targetDegrees = value;
      Serial.print("Motor B target: "); Serial.println(value);
    }
  }

  unsigned long now = micros();
  float dt = (now - lastLoopMicros) / 1000000.0f;
  lastLoopMicros = now;

  float posA = axisUpdatePosition(motorA);
  float outA = axisPidUpdate(motorA, posA, dt);
  axisDriveMotor(motorA, outA);

  float posB = axisUpdatePosition(motorB);
  float outB = axisPidUpdate(motorB, posB, dt);
  axisDriveMotor(motorB, outB);

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 100) {
    lastPrint = millis();
    Serial.print("A: tgt="); Serial.print(motorA.targetDegrees);
    Serial.print(" pos="); Serial.print(posA);
    Serial.print(" out="); Serial.print(outA);
    Serial.print("  |  B: tgt="); Serial.print(motorB.targetDegrees);
    Serial.print(" pos="); Serial.print(posB);
    Serial.print(" out="); Serial.println(outB);
  }

  delay(5); // ~200Hz control loop
}