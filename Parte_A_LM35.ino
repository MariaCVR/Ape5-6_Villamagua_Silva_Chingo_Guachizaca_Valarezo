const int PIN_LM35 = 34;
const int N = 10; // Cambiar a 1, 10 y 30 para las pruebas
const unsigned long PERIODO_MUESTREO_MS = 100;

float bufferCircular[30]; // Tamaño máximo a usar
int indice = 0;
int muestrasValidas = 0;
unsigned long ultimoMuestreo = 0;

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_0db);
  analogReadMilliVolts(PIN_LM35); // Lectura inicial de estabilización
}

void loop() {
  unsigned long ahora = millis();
  if (ahora - ultimoMuestreo >= PERIODO_MUESTREO_MS) {
    ultimoMuestreo = ahora;

    // 1. Leer milivoltios y convertir a °C
    uint32_t mv = analogReadMilliVolts(PIN_LM35);
    float tempCruda = mv / 10.0f;

    // 2. Guardar en búfer circular
    bufferCircular[indice] = tempCruda;
    indice = (indice + 1) % N;
    if (muestrasValidas < N) {
      muestrasValidas++;
    }

    // 3. Calcular media móvil
    float suma = 0.0f;
    for (int i = 0; i < muestrasValidas; i++) {
      suma += bufferCircular[i];
    }
    float tempFiltrada = suma / muestrasValidas;

    // 4. Formato exacto para Serial Plotter
    Serial.print("T_cruda:");
    Serial.print(tempCruda, 2);
    Serial.print(",T_filtrada:");
    Serial.println(tempFiltrada, 2);
  }
}
