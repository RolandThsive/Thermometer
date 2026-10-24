#include "sht40sensor.h"
#include <math.h>

// Sensirion CRC-8 (polynomial 0x31, initial value 0xFF) used by the SHT40
// to protect each 16-bit word it sends over I2C.
static uint8_t sht40CRC8(uint8_t msb, uint8_t lsb) {
    uint8_t data[2] = { msb, lsb };
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < 2; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) crc = (uint8_t)((crc << 1) ^ 0x31);
            else             crc = (uint8_t)(crc << 1);
        }
    }
    return crc;
}

bool readSHT40(uint8_t channel, float &tempC, float &humidity) {
    if (!selectPCAChannel(channel)) return false;

    Wire.beginTransmission(SHT40_ADDR);
    Wire.write(0xFD); // High precision command
    if (Wire.endTransmission() != 0) return false;

    delay(10); // Measurement delay

    uint8_t bytesReceived = Wire.requestFrom((uint8_t)SHT40_ADDR, (uint8_t)6);
    if (bytesReceived == 6 && Wire.available() >= 6) {
        uint8_t t_msb  = Wire.read();
        uint8_t t_lsb  = Wire.read();
        uint8_t t_crc  = Wire.read();
        uint8_t rh_msb = Wire.read();
        uint8_t rh_lsb = Wire.read();
        uint8_t rh_crc = Wire.read();

        // Validate both CRCs before trusting the reading - a corrupted I2C
        // transaction must never be allowed to drive the motors.
        if (sht40CRC8(t_msb, t_lsb) != t_crc || sht40CRC8(rh_msb, rh_lsb) != rh_crc) {
            return false;
        }

        uint16_t t_raw  = ((uint16_t)t_msb << 8)  | t_lsb;
        uint16_t rh_raw = ((uint16_t)rh_msb << 8) | rh_lsb;

        tempC = -45.0f + 175.0f * ((float)t_raw / 65535.0f);
        humidity = -6.0f + 125.0f * ((float)rh_raw / 65535.0f);

        if (humidity < 0.0f) humidity = 0.0f;
        if (humidity > 100.0f) humidity = 100.0f;
        return true;
    }

    // Discard any stray bytes left in the I2C buffer on a short/failed
    // transaction so the next request starts from a known state.
    while (Wire.available() > 0) Wire.read();
    return false;
}

// Magnus-Tetens approximation for Dew Point
float calculateDewPoint(float tempC, float humidity) {
    const float a = 17.27f;
    const float b = 237.7f;

    // Guard against log(0)/log(negative) and a near-zero denominator, which
    // would otherwise produce NaN/Inf and send an invalid position downstream.
    float safeHumidity = humidity < 0.1f ? 0.1f : humidity;

    float alpha = ((a * tempC) / (b + tempC)) + logf(safeHumidity / 100.0f);
    float denom = (a - alpha);
    if (fabsf(denom) < 1e-6f) denom = (denom < 0.0f) ? -1e-6f : 1e-6f;

    return (b * alpha) / denom;
}
