#ifndef AS5600_H
#define AS5600_H

#include <Arduino.h>
#include <Wire.h>

#define PCA9546A_ADDR 0x70
#define AS5600_ADDR   0x36

bool selectPCAChannel(uint8_t channel);
void disableAllPCAChannels();
uint16_t readAS5600RawAngle(uint8_t channel);

#endif