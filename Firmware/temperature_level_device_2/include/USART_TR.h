#ifndef USART_TR_H
#define USART_TR_H

#include <Arduino.h>

void initializeUSART(unsigned long baudRate);
void sendUSART(uint8_t data);
void sendFloatData(float value);
char receiveUSART();
bool readFloatsUSART(float &val1, float &val2);

#endif
