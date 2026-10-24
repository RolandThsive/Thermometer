#include "AS5600.h"

bool selectPCAChannel(uint8_t channel) {
    if (channel > 3) return false;
    Wire.beginTransmission(PCA9546A_ADDR);
    Wire.write(1 << channel);
    return (Wire.endTransmission() == 0);
}

void disableAllPCAChannels() {
    Wire.beginTransmission(PCA9546A_ADDR);
    Wire.write(0x00);
    Wire.endTransmission();
}

uint16_t readAS5600RawAngle(uint8_t channel) {
    if (!selectPCAChannel(channel)) return 0xFFFF;

    Wire.beginTransmission(AS5600_ADDR);
    Wire.write(0x0C); // RAW ANGLE Register MSB
    if (Wire.endTransmission() != 0) return 0xFFFF;

    uint8_t bytesReceived = Wire.requestFrom((uint8_t)AS5600_ADDR, (uint8_t)2);
    if (bytesReceived == 2 && Wire.available() >= 2) {
        uint8_t highByte = Wire.read();
        uint8_t lowByte  = Wire.read();
        return ((uint16_t)highByte << 8) | lowByte;
    }

    // Discard any stray bytes left in the I2C buffer on a short/failed
    // transaction so the next request starts from a known state instead
    // of silently desyncing.
    while (Wire.available() > 0) Wire.read();
    return 0xFFFF;
}
