#include <Arduino.h>

// AP05-AP06 - Parte A: LM35 y media movil.
constexpr uint8_t PIN_LM35 = 34;
constexpr uint32_t TS = 100;  // ms
constexpr size_t N = 10;      // Cambiar a 1, 10 o 30.
static_assert(N > 0, "N debe ser positivo");

float buffer[N] = {};
size_t indice = 0, cantidad = 0;
float suma = 0.0f;
uint32_t ultimaMuestra = 0;

// Media movil: suma acumulada y reemplazo circular.
float mediaMovil(float nueva) {
  if (cantidad == N) {
    suma -= buffer[indice];  // Retirar la muestra mas antigua.
  } else {
    cantidad++;             // Arranque sin promediar ceros.
  }
  buffer[indice] = nueva;
  suma += nueva;
  indice = (indice + 1) % N;
  return suma / cantidad;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_0db);
  ultimaMuestra = millis();
}

void loop() {
  uint32_t ahora = millis();
  if (ahora - ultimaMuestra >= TS) {
    ultimaMuestra = ahora;
    // LM35: 10 mV por grado Celsius.
    float cruda = analogReadMilliVolts(PIN_LM35) / 10.0f;
    float filtrada = mediaMovil(cruda);
    Serial.print("T_cruda:");
    Serial.print(cruda, 2);
    Serial.print(",T_filtrada:");
    Serial.println(filtrada, 2);
  }
}
