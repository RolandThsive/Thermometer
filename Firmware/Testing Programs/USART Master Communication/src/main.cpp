#include <Arduino.h>

// Waveshare ESP32-S3-Zero Pin Mapping
// ESP32-S3-Zero TX (GPIO 4) -> ESP32-C3 RX (D7 / GPIO 7)
// ESP32-S3-Zero RX (GPIO 5) -> ESP32-C3 TX (D6 / GPIO 6)
#define S3_RX_PIN 4
#define S3_TX_PIN 5
#define BAUD_RATE 115200

// Unpack 32-bit int from 4-byte frame into float
float parseBinaryFloat(int32_t rawVal) {
    return (float)rawVal / 10000.0f;
}

// Blocks until two valid 5-byte binary float frames arrive, re-sending 't' every 5 seconds if no payload received
void waitForBinaryFloatPayload(float &temp, float &dewPoint) {
    uint8_t buffer[10];
    unsigned long lastSendTime = millis();

    Serial.println("      -> Waiting for binary payload (will retry every 5s)...");

    while (true) {
        // Wait until at least 10 bytes are available in the serial buffer
        if (Serial1.available() >= 10) {
            // Peek at the first byte to verify frame header
            if (Serial1.peek() != 0xAA) {
                // Drop byte if it doesn't match start marker (sync recovery)
                Serial1.read();
                continue;
            }

            // Read 10 bytes into buffer
            Serial1.readBytes(buffer, 10);

            // Verify both frame headers (buffer[0] and buffer[5])
            if (buffer[0] == 0xAA && buffer[5] == 0xAA) {
                int32_t rawTemp = ((int32_t)buffer[1] << 24) |
                                  ((int32_t)buffer[2] << 16) |
                                  ((int32_t)buffer[3] << 8)  |
                                  (int32_t)buffer[4];

                int32_t rawDew =  ((int32_t)buffer[6] << 24) |
                                  ((int32_t)buffer[7] << 16) |
                                  ((int32_t)buffer[8] << 8)  |
                                  (int32_t)buffer[9];

                temp = parseBinaryFloat(rawTemp);
                dewPoint = parseBinaryFloat(rawDew);
                return; // Valid payload received and parsed, exit function
            }
        }

        // Check if 5 seconds have passed without receiving a valid payload
        if (millis() - lastSendTime >= 5000) {
            Serial.println("      -> [TIMEOUT] No response in 5s. Re-sending 't'...");
            Serial1.write('t');
            lastSendTime = millis(); // Reset timer
        }

        delay(10); // Small yield to prevent watchdog resets while waiting
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    // Initialize UART1 on Waveshare ESP32-S3-Zero pins
    Serial1.begin(BAUD_RATE, SERIAL_8N1, S3_RX_PIN, S3_TX_PIN);

    Serial.println("\n--- ESP32-S3 Master Controller (Auto-Retry Receive) ---");
}

void loop() {
    float rxTemp = 0.0f;
    float rxDewPoint = 0.0f;

    // ----------------------------------------------------
    // STEP 1: Send 't' command
    // ----------------------------------------------------
    Serial.println("[1/4] Sending 't' command...");
    Serial1.write('t');

    // ----------------------------------------------------
    // STEP 2: Wait until binary payload is received (retries every 5s)
    // ----------------------------------------------------
    waitForBinaryFloatPayload(rxTemp, rxDewPoint);

    Serial.print("      -> Received Temperature: ");
    Serial.print(rxTemp, 4);
    Serial.println(" C");

    Serial.print("      -> Received Dew Point:   ");
    Serial.print(rxDewPoint, 4);
    Serial.println(" C");

    delay(1000);

    // ----------------------------------------------------
    // STEP 3: Send 'w' command
    // ----------------------------------------------------
    Serial.println("[2/4] Sending 'w' command...");
    Serial1.write('w');
    delay(1000);

    // ----------------------------------------------------
    // STEP 4: Send 'm 15.0 20.0' command
    // ----------------------------------------------------
    /*Serial.println("[3/4] Sending 'm 15.0 20.0' command...");
    Serial1.println("m 15.0 20.0"); // Includes '\r\n' for sscanf line parsing

    Serial.println("[4/4] Sequence completed. Waiting 5 seconds...\n");
    delay(5000);*/
}