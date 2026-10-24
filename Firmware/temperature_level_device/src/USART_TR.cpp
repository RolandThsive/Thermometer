#include "USART_TR.h"

void initializeUSART(unsigned long baudRate) {
    // D7 = RX (GPIO7), D6 = TX (GPIO6)
    Serial1.begin(baudRate, SERIAL_8N1, D7, D6); 
}

void sendUSART(uint8_t data) {
    Serial1.write(data);
}

void sendFloatData(float value) {
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