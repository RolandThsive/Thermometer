#include "main.h"
#include "debug_utils.h"

float currentTemp = 0.0f;
float currentDewPoint = 0.0f;
float humidity = 0.0f;
int batteryMode = 0;

void setup() {
    DEBUG_BEGIN(115200);
    // Hardware Initializations
    initializeUSART(115200);
    motorInitialize();
    pinMode(GPIO_PIN_10, INPUT_PULLDOWN);
    // Wire setup for Seeed Studio ESP32-C3
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);
    Wire.setTimeOut(50); // ms - bounds any I2C fault instead of hanging forever

    DEBUG_PRINTLN("\n=== ESP32-C3 AWAKE ===");
}

void loop() {
    char cmd = receiveUSART();

    batteryMode = digitalRead(GPIO_PIN_10);

    if (cmd != '\0' && batteryMode == 1) {
        if (cmd == 't') {
            // Read SHT40 Sensor on Channel 2
            if (readSHT40(SHT40_PCA_CHANNEL, currentTemp, humidity)) {
                currentDewPoint = calculateDewPoint(currentTemp, humidity);
            }

            // Transmit binary payload back over USART
            sendFloatData(currentTemp);
            sendFloatData(currentDewPoint);
        } 
        else if (cmd == 'w') {
            // Actuate Motor 1 (Prong 1) to Current Temperature using fixed 300 deg span
            moveProngToPosition(M1, currentTemp);

            // Actuate Motor 2 (Prong 2) to Dew Point using fixed 300 deg span
            moveProngToPosition(M2, currentDewPoint);
        }
        else if (cmd == 'm') {
            float arg1 = 0.0f; // TEMPERATURE
            float arg2 = 0.0f; // DEW POINT TEMPERATURE
            // Reads "<float1> <float2>\n" following 'm'
            if (readFloatsUSART(arg1, arg2)) {
                    if((arg1 > 100.0f || arg1 < 0.0f) || (arg2 > 100.0f || arg2 < 0.0f)){
                    arg1 = 0.0f;
                    arg2 = 0.0f;
                }
                // Update temperatures directly or drive prongs to positions
                moveProngToPosition(M1, arg1);
                moveProngToPosition(M2, arg2);
            }
        }
    }
    else if(batteryMode == 0){
        if (readSHT40(SHT40_PCA_CHANNEL, currentTemp, humidity)) {
                currentDewPoint = calculateDewPoint(currentTemp, humidity);
            }
            // Actuate Motor 1 (Prong 1) to Current Temperature using fixed 300 deg span
            moveProngToPosition(M1, currentTemp);

            // Actuate Motor 2 (Prong 2) to Dew Point using fixed 300 deg span
            moveProngToPosition(M2, currentDewPoint);
            delay(1000);
    }   
}
