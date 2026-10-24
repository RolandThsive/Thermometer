#ifndef SHT40SENSOR_H
#define SHT40SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "AS5600.h"

#define SHT40_ADDR 0x44

bool readSHT40(uint8_t channel, float &tempC, float &humidity);
float calculateDewPoint(float tempC, float humidity);

#endif
