#include <Arduino.h>
#include <Wire.h>

// ESP32-C3 Zero Pin Mapping
#define SDA_PIN 6
#define SCL_PIN 7

// I2C Addresses
#define PCA9546A_ADDR 0x70
#define AS5600_ADDR   0x36
#define SHT40_ADDR    0x44

// AS5600 Registers
#define AS5600_RAW_ANGLE_REG 0x0C
#define AS5600_STATUS_REG    0x0B

// SHT40 Commands
#define SHT40_MEAS_HIGH_PRECISION 0xFD

// Selects a single channel (0 to 3) on the PCA9546A
bool selectPCAChannel(uint8_t channel) {
  if (channel > 3) return false;

  Wire.beginTransmission(PCA9546A_ADDR);
  Wire.write(1 << channel); // Bitmask: Ch0=0x01, Ch1=0x02, Ch2=0x04, Ch3=0x08
  return (Wire.endTransmission() == 0);
}

// Disables all channels on PCA9546A
void disableAllPCAChannels() {
  Wire.beginTransmission(PCA9546A_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();
}

// Reads Raw Angle (0 - 4095) from AS5600
uint16_t readAS5600RawAngle() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(AS5600_RAW_ANGLE_REG);
  if (Wire.endTransmission() != 0) return 0xFFFF;

  Wire.requestFrom((uint8_t)AS5600_ADDR, (uint8_t)2);
  if (Wire.available() == 2) {
    uint8_t highByte = Wire.read();
    uint8_t lowByte  = Wire.read();
    return ((uint16_t)highByte << 8) | lowByte;
  }
  return 0xFFFF;
}

// Reads Magnet Status from AS5600 (Bit 5 = Magnet Detected)
uint8_t readAS5600Status() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(AS5600_STATUS_REG);
  if (Wire.endTransmission() == 0) {
    Wire.requestFrom((uint8_t)AS5600_ADDR, (uint8_t)1);
    if (Wire.available()) return Wire.read();
  }
  return 0x00;
}

// Reads Temperature (°C) and Humidity (%RH) from SHT40
bool readSHT40(float &tempC, float &humidity) {
  Wire.beginTransmission(SHT40_ADDR);
  Wire.write(SHT40_MEAS_HIGH_PRECISION);
  if (Wire.endTransmission() != 0) return false;

  delay(10); // Measurement delay

  Wire.requestFrom((uint8_t)SHT40_ADDR, (uint8_t)6);
  if (Wire.available() == 6) {
    uint16_t t_raw = ((uint16_t)Wire.read() << 8) | Wire.read();
    Wire.read(); // Skip Temp CRC
    uint16_t rh_raw = ((uint16_t)Wire.read() << 8) | Wire.read();
    Wire.read(); // Skip RH CRC

    tempC = -45.0 + 175.0 * ((float)t_raw / 65535.0);
    humidity = -6.0 + 125.0 * ((float)rh_raw / 65535.0);

    if (humidity < 0.0) humidity = 0.0;
    if (humidity > 100.0) humidity = 100.0;
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n==============================================");
  Serial.println("  PCA9546A Multi-Sensor Monitoring Dashboard  ");
  Serial.println("==============================================");

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000);
  Wire.setTimeOut(200);

  Wire.beginTransmission(PCA9546A_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.printf("ERROR: Could not find PCA9546A at address 0x%02X!\n", PCA9546A_ADDR);
    while (1) delay(1000);
  }
  Serial.printf("PCA9546A multiplexer online at 0x%02X.\n\n", PCA9546A_ADDR);
}

void loop() {
  Serial.println("---------------- SCAN START ----------------");

  for (uint8_t ch = 0; ch < 4; ch++) {
    Serial.printf("[ Channel %d ]\n", ch);

    if (selectPCAChannel(ch)) {
      delay(5);
      uint8_t devicesFound = 0;

      for (uint8_t addr = 1; addr < 127; addr++) {
        if (addr == PCA9546A_ADDR) continue;

        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
          devicesFound++;

          // AS5600 Handler (Channels 1 & 3)
          if (addr == AS5600_ADDR) {
            uint8_t status = readAS5600Status();
            bool magnetDetected = (status & 0x20);

            if (magnetDetected) {
              uint16_t rawAngle = readAS5600RawAngle();
              if (rawAngle != 0xFFFF) {
                float deg = (rawAngle * 360.0) / 4096.0;
                Serial.printf("  + AS5600 Encoder [0x36] -> Raw: %4d / 4095 | Angle: %6.2f°\n", rawAngle, deg);
              } else {
                Serial.println("  + AS5600 Encoder [0x36] -> Read error!");
              }
            } else {
              Serial.println("  + AS5600 Encoder [0x36] -> Warning: Magnet not detected!");
            }
          }
          // SHT40 Handler (Channel 2)
          else if (addr == SHT40_ADDR) {
            float temp, rh;
            if (readSHT40(temp, rh)) {
              Serial.printf("  + SHT40 Sensor   [0x44] -> Temp: %5.2f °C | Humidity: %5.2f %%RH\n", temp, rh);
            } else {
              Serial.println("  + SHT40 Sensor   [0x44] -> Measurement failed!");
            }
          }
          // Generic Device Handler
          else {
            Serial.printf("  + Generic Device detected at 0x%02X\n", addr);
          }
        }
      }

      if (devicesFound == 0) {
        Serial.println("  - Empty / No devices detected.");
      }
    } else {
      Serial.printf("  ! Failed to switch to Channel %d.\n", ch);
    }
  }

  disableAllPCAChannels();
  Serial.println("--------------------------------------------\n");
  delay(1000);
}