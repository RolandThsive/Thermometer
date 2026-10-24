#include "USART_TR.h"

void initializeUSART(unsigned long baudRate) {
    // D7 = RX (GPIO7), D6 = TX (GPIO6)
    Serial1.begin(baudRate, SERIAL_8N1, D7, D6); 
}

void sendUSART(uint8_t data) {
    Serial1.write(data);
}

void sendFloatData(float value) {
    // NOTE: this frame has a 0xAA header byte but no escaping/checksum, so a
    // data byte that happens to equal 0xAA can look like the start of the
    // next frame if a byte is ever dropped on the wire. Left as-is here
    // since changing the frame format would require updating the receiver
    // (not part of this codebase) to match - flagging for awareness only.
    int32_t processedData = (int32_t)(value * 10000.0f);
    
    sendUSART(0xAA); // Header Frame
    sendUSART((processedData >> 24) & 0xFF);
    sendUSART((processedData >> 16) & 0xFF);
    sendUSART((processedData >> 8)  & 0xFF);
    sendUSART(processedData & 0xFF);
}

char receiveUSART() {
    if (Serial1.available() > 0) {
        return (char)Serial1.read();
    }
    return '\0'; 
}

// Helper function to read the remaining floats after 'm' is detected
bool readFloatsUSART(float &val1, float &val2) {
    // Wait briefly for the rest of the line if bytes are still transmitting
    unsigned long startTime = millis();
    while (Serial1.available() < 3 && millis() - startTime < 100) {
        // Wait up to 100ms for incoming string bytes
    }

    // Read the rest of the line until newline
    String restOfLine = Serial1.readStringUntil('\n');
    restOfLine.trim(); // Clean trailing \r or spaces

    // Parse the two floating point numbers from the string
    if (sscanf(restOfLine.c_str(), "%f %f", &val1, &val2) == 2) {
        return true;
    }
    return false;
}