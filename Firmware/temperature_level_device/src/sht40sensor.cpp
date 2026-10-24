#include "sht40sensor.h"
#include <math.h>

bool readSHT40(uint8_t channel, float &tempC, float &humidity) {
    if (!selectPCAChannel(channel)) return false;

    Wire.beginTransmission(SHT40_ADDR);
    Wire.write(0xFD); // High precision command
    if (Wire.endTransmission() != 0) return false;

    delay(10); // Measurement delay

    Wire.requestFrom((uint8_t)SHT40_ADDR, (uint8_t)6);
    if (Wire.available() == 6) {
        uint16_t t_raw = ((uint16_t)Wire.read() << 8) | Wire.read();
        Wire.read(); // Skip CRC
        uint16_t rh_raw = ((uint16_t)Wire.read() << 8) | Wire.read();
        Wire.read(); // Skip CRC

        tempC = -45.0f + 175.0f * ((float)t_raw / 65535.0f);
        humidity = -6.0f + 125.0f * ((float)rh_raw / 65535.0f);

        if (humidity < 0.0f) humidity = 0.0f;
        if (humidity > 100.0f) humidity = 100.0f;
        return true;
    }
    return false;
}

// Magnus-Tetens approximation for Dew Point
float calculateDewPoint(float tempC, float humidity) {
    const float a = 17.27f;
    const float b = 237.7f;
    float alpha = ((a * tempC) / (b + tempC)) + logf(humidity / 100.0f);
    return (b * alpha) / (a - alpha);
}