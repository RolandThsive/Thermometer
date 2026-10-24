#include "main.h"
#include "debug_utils.h"

float currentTemp = 0.0f;
float currentDewPoint = 0.0f;


void setup() {
    DEBUG_BEGIN(115200);

    // Hardware Initializations
    initializeUSART(115200);
    motorInitialize();

    // Wire setup for Seeed Studio ESP32-C3
    Wire.begin(D4, D5);
    Wire.setClock(400000);

    DEBUG_PRINTLN("\n=== ESP32-C3 AWAKE ===");
}

void loop() {
    char cmd = receiveUSART();

    if (cmd != '\0') {
        if (cmd == 't') {
            // Read SHT40 Sensor on Channel 2
            float temp = 0.0f, humidity = 0.0f;
            if (readSHT40(SHT40_PCA_CHANNEL, temp, humidity)) {
                currentTemp = temp;
                currentDewPoint = calculateDewPoint(temp, humidity);
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
    }
    delay(4000);
    float temp = 0.0f, humidity = 0.0f;

    readSHT40(SHT40_PCA_CHANNEL, temp, humidity);
    currentTemp = temp;
    currentDewPoint = calculateDewPoint(temp, humidity);
    moveProngToPosition(M1, currentTemp);
    moveProngToPosition(M2, currentDewPoint);


    delay(1000);

}