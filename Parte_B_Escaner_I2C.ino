#include <Arduino.h>
#include <Wire.h>

// AP05-AP06 - Parte B: escaner de direcciones I2C.
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);         // SDA, SCL.
  Wire.setClock(100000);      // I2C estandar: 100 kHz.
}

void loop() {
  Serial.println("Escaneando bus I2C...");
  int encontrados = 0;
  for (uint8_t dir = 1; dir <= 126; dir++) {
    Wire.beginTransmission(dir);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Dispositivo encontrado: 0x");
      if (dir < 16) Serial.print('0');
      Serial.println(dir, HEX);
      encontrados++;
    } else if (error == 4 || error == 5) {
      Serial.print("Error de bus en 0x");
      Serial.println(dir, HEX);
    }
  }
  Serial.print("Total de dispositivos: ");
  Serial.println(encontrados);
  delay(3000);  // Pausa solo en el escaner independiente.
}
