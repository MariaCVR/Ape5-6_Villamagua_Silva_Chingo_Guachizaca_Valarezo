#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA = GPIO21, SCL = GPIO22
  Serial.println("Escaneando bus I2C...");

  int encontrados = 0;
  for (uint8_t dir = 1; dir <= 126; dir++) {
    Wire.beginTransmission(dir);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Dispositivo encontrado en 0x");
      if (dir < 16) Serial.print("0");
      Serial.println(dir, HEX);
      encontrados++;
    }
  }

  if (encontrados == 0) {
    Serial.println("No se encontraron dispositivos I2C. Revisa SDA/SCL/VCC/GND.");
  }
}

void loop() {
  // No se requiere acción continua
}
