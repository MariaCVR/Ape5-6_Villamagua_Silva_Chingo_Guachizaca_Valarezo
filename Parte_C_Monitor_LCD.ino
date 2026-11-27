#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <stdio.h>

// AP05-AP06 - Parte C: monitor con LCD y alarma.
constexpr uint8_t PIN_LM35 = 34;
constexpr uint32_t TS = 100;  // ms
constexpr size_t N = 10;      // Cambiar a 1, 10 o 30.
static_assert(N > 0, "N debe ser positivo");

float buffer[N] = {};
size_t indice = 0, cantidad = 0;
float suma = 0.0f;
uint32_t ultimaMuestra = 0;
constexpr uint8_t PIN_LED = 19;
constexpr uint8_t LCD_DIR = 0x27;  // Cambiar segun la Parte B.
constexpr uint32_t TLCD = 500;     // ms
constexpr float T_ON = 28.0f;
constexpr float T_OFF = 26.0f;
static_assert(T_ON > T_OFF, "Umbrales incorrectos");

LiquidCrystal_I2C lcd(LCD_DIR, 16, 2);
uint32_t ultimaLCD = 0;
float filtrada = 0.0f;
bool alarma = false;

// Caracter personalizado del simbolo de grado (5 x 8).
uint8_t grado[8] = {0b00110, 0b01001, 0b01001, 0b00110,
                    0b00000, 0b00000, 0b00000, 0b00000};

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

void controlarAlarma(float temperatura) {
  if (!alarma && temperatura >= T_ON) {
    alarma = true;
  } else if (alarma && temperatura <= T_OFF) {
    alarma = false;
  }
  digitalWrite(PIN_LED, alarma ? HIGH : LOW);
}

void actualizarLCD() {
  char fila[17];  // 16 caracteres y terminador nulo.
  // %5.1f asegura un ancho minimo de cinco caracteres.
  snprintf(fila, sizeof(fila), "Temp:%5.1f", filtrada);
  lcd.setCursor(0, 0);
  lcd.print(fila);            // 10 caracteres en el rango del ADC.
  lcd.write((uint8_t)0);      // Grado personalizado.
  lcd.print("C    ");        // Completa los 16 caracteres.
  lcd.setCursor(0, 1);
  lcd.print(alarma ? "Alarma: SI      " : "Alarma: NO      ");
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  analogReadResolution(12);
  analogSetAttenuation(ADC_0db);
  Wire.begin(21, 22);
  Wire.setClock(100000);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, grado);
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...    ");
  lcd.setCursor(0, 1);
  lcd.print("Alarma: NO      ");
  ultimaMuestra = ultimaLCD = millis();
}

void loop() {
  uint32_t ahora = millis();
  if (ahora - ultimaMuestra >= TS) {
    ultimaMuestra = ahora;
    float cruda = analogReadMilliVolts(PIN_LM35) / 10.0f;
    filtrada = mediaMovil(cruda);
    controlarAlarma(filtrada);
    Serial.print("T_cruda:");
    Serial.print(cruda, 2);
    Serial.print(",T_filtrada:");
    Serial.print(filtrada, 2);
    Serial.print(",Alarma:");
    Serial.println(alarma ? 1 : 0);
  }
  if (cantidad > 0 && ahora - ultimaLCD >= TLCD) {
    ultimaLCD = ahora;
    actualizarLCD();
  }
}
