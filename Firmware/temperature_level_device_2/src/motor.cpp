#include "motor.h"
#include "debug_utils.h"

// Define motor parameters using the MotorParam struct
MotorParam M1 = {
    .MotorNumber = 1,
    .M_ZERO = 850,
    .EncoderDir = 1,
    .MotorDir = -1,
    .PCAChannel = M2_AS5600_CHANNEL,
    .Motor_Pin1 = D0,
    .Motor_Pin2 = D1,
    .MotorChannel_Pin1 = 0,
    .MotorChannel_Pin2 = 1

};

MotorParam M2 = {
    .MotorNumber = 2,
    .M_ZERO = 4085,
    .EncoderDir = 1,
    .MotorDir = -1,
    .PCAChannel = M1_AS5600_CHANNEL,
    .Motor_Pin1 = D2,
    .Motor_Pin2 = D3,
    .MotorChannel_Pin1 = 2,
    .MotorChannel_Pin2 = 3
};


void motorInitialize() {

    pinMode(SLEEP_PIN, OUTPUT);
    motorWake();
    
     // Configure each PWM channel at 25kHz / 8-bit resolution
    ledcSetup(CH_AIN1, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
    ledcSetup(CH_AIN2, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
    ledcSetup(CH_BIN1, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
    ledcSetup(CH_BIN2, PWM_FREQ_HZ, PWM_RESOLUTION_BITS);
    
    // Attach each channel to its physical pin
    ledcAttachPin(M1.Motor_Pin1, CH_AIN1);
    ledcAttachPin(M1.Motor_Pin2, CH_AIN2);
    ledcAttachPin(M2.Motor_Pin1, CH_BIN1);
    ledcAttachPin(M2.Motor_Pin2, CH_BIN2);

    motorStop(M1);
    motorStop(M2);
}

void motorSleep() {
    digitalWrite(SLEEP_PIN, LOW);
}

void motorWake() {
    digitalWrite(SLEEP_PIN, HIGH);
    delay(2);
}

void motorStop(const MotorParam &Motor) {
    
    ledcWrite(Motor.MotorChannel_Pin1, 0);
    ledcWrite(Motor.MotorChannel_Pin2, 0);
}

void motorDrive(const MotorParam &Motor, double controlOutput, double currentError) {
    if (abs(currentError) < DEADBAND) {
        motorStop(Motor);
        return;
    }

    bool forward = (controlOutput > 0);
    controlOutput = constrain(abs(controlOutput), MIN_PWM, MAX_PWM);

    if ((forward)) {
        ledcWrite(Motor.MotorChannel_Pin1, controlOutput);
        ledcWrite(Motor.MotorChannel_Pin2, 0);
    } else {
        ledcWrite(Motor.MotorChannel_Pin1, 0);
        ledcWrite(Motor.MotorChannel_Pin2, controlOutput);
    }
    
}

double as5600CalculateShortestError(double target, double actual) {
    // Direct linear error along the valid 0..TOTAL_SPAN_COUNTS path
    return target - actual;
}

int virtualAngleCalc(int rawAngle, const MotorParam &Motor) {
    if (rawAngle == -1 || rawAngle == 0xFFFF) return -1;

    int zeroPos = Motor.M_ZERO;
    int encoderDirection = Motor.EncoderDir;
    // Shift coordinate system so physical zero is at 0

    if(encoderDirection == -1){
        int virtualAngle = rawAngle - zeroPos;

        if (virtualAngle < 0) {
            virtualAngle += 4096;
        }
        DEBUG_PRINT("RAW-1: ");
        DEBUG_PRINT(rawAngle);
        DEBUG_PRINT(" VirtualAngle: ");
        DEBUG_PRINTLN(virtualAngle);
        return virtualAngle;
    }
    else{
        int virtualAngle = zeroPos - rawAngle;

        if (virtualAngle < 0) {
            virtualAngle += 4096;
        }
        return virtualAngle;
    }
}

void runPositioningLoop(const MotorParam &Motor, float targetPos) {
    int pcaChannel = Motor.PCAChannel;
    int zeroPos = Motor.M_ZERO;
    int motorNum = Motor.MotorNumber;
    
    int settledCycles = 0;
    double integral = 0.0;
    double lastError = 0.0;
    double input = 0.0;
    uint8_t consecutiveFaults = 0;

    unsigned long lastTime = millis();
    unsigned long safetyTimeout = millis(); 

    while (settledCycles < PID_CYCLES && (millis() - safetyTimeout < PID_SAFETY_TIMEOUT)) {
        unsigned long now = millis();
        double dt = (now - lastTime) / 1000.0;
        if (dt <= 0.0) dt = 0.01;

        uint16_t rawAngle = readAS5600RawAngle(pcaChannel);
        if (rawAngle != 0xFFFF) {
            int virtAngle = virtualAngleCalc((int)rawAngle, Motor);
                input = (double)virtAngle;
            consecutiveFaults = 0;
        } else {
            consecutiveFaults++;
            // Encoder is unreadable (I2C fault) - abort the move on a
            // stale input rather than continuing to drive blind.
            if (consecutiveFaults >= MAX_CONSECUTIVE_SENSOR_FAULTS) {
                DEBUG_PRINTLN("Encoder read failed repeatedly - aborting move.");
                break;
            }
        }
        
        double error = input - targetPos;

        DEBUG_PRINT("RAW: ");
        DEBUG_PRINT(rawAngle);
        DEBUG_PRINT(" | input: ");
        DEBUG_PRINT(input);
        DEBUG_PRINT(" | targetPos: ");
        DEBUG_PRINTLN(targetPos);

        if (abs(error) < DEADBAND) {
            settledCycles++;
        } else {
            settledCycles = 0; 
        }

        integral += error * dt;
        integral = constrain(integral, -100.0, 100.0);
        double derivative = (error - lastError) / dt;

        double output = (Kp * error) + (Ki * integral) + (Kd * derivative);

        lastError = error;
        lastTime = now;

        motorDrive(Motor, output, error);

        delay(20); 
    }

    motorStop(Motor);
}

void moveProngToPosition(const MotorParam &Motor, float targetValue) { 
    targetValue = constrain(targetValue, 0.0f, 100.0f);
    double ratio = (double)targetValue / 100.0;

    double targetPos = ratio * (double)TOTAL_SPAN_COUNTS;
    targetPos = constrain(targetPos, 0.0, (double)TOTAL_SPAN_COUNTS);
    
    DEBUG_PRINT(" TargetPOS: ");
    DEBUG_PRINTLN(targetPos);
    runPositioningLoop(Motor, targetPos);
}
