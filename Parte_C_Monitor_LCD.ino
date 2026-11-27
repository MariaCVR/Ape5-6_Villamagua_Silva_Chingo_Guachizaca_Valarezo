#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int PIN_LM35 = 34;
const int PIN_LED  = 19;
const uint8_t DIR_LCD = 0x27; // Cambiar a 0x3F si tu escáner mostró esa dirección

const int N = 10;
const unsigned long PERIODO_MUESTREO_MS = 100;
const unsigned long PERIODO_LCD_MS      = 500;

// Umbrales de histéresis (ajustar a 27.0 / 25.0 si no alcanzas 28 °C)
const float T_ON  = 28.0f;
const float T_OFF = 26.0f;

LiquidCrystal_I2C lcd(DIR_LCD, 16, 2);

// Carácter personalizado para el símbolo de grado (°)
byte gradoChar[8] = {
  0b00110,
  0b01001,
  0b01001,
  0b00110,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

float bufferCircular[N];
int indice = 0;
int muestrasValidas = 0;
bool alarmaActiva = false;

unsigned long ultimoMuestreo = 0;
unsigned long ultimaLcd = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  analogSetAttenuation(ADC_0db);
  analogReadMilliVolts(PIN_LM35);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, gradoChar);
}

void loop() {
  unsigned long ahora = millis();

  // Muestreo y filtro cada 100 ms
  if (ahora - ultimoMuestreo >= PERIODO_MUESTREO_MS) {
    ultimoMuestreo = ahora;

    uint32_t mv = analogReadMilliVolts(PIN_LM35);
    float tempCruda = mv / 10.0f;

    bufferCircular[indice] = tempCruda;
    indice = (indice + 1) % N;
    if (muestrasValidas < N) {
      muestrasValidas++;
    }

    float suma = 0.0f;
    for (int i = 0; i < muestrasValidas; i++) {
      suma += bufferCircular[i];
    }
    float tempFiltrada = suma / muestrasValidas;

    // Lógica de histéresis
    if (!alarmaActiva && tempFiltrada >= T_ON) {
      alarmaActiva = true;
    } else if (alarmaActiva && tempFiltrada <= T_OFF) {
      alarmaActiva = false;
    }

    digitalWrite(PIN_LED, alarmaActiva ? HIGH : LOW);

    // Salida serie para monitoreo
    Serial.print("T_cruda:");
    Serial.print(tempCruda, 2);
    Serial.print(",T_filtrada:");
    Serial.print(tempFiltrada, 2);
    Serial.print(",Alarma:");
    Serial.println(alarmaActiva ? 1 : 0);

    // Actualizar LCD cada 500 ms sin lcd.clear()
    if (ahora - ultimaLcd >= PERIODO_LCD_MS) {
      ultimaLcd = ahora;

      // Fila 0: Temp:XX.X°C
      lcd.setCursor(0, 0);
      lcd.print("Temp:");
      lcd.print(tempFiltrada, 1);
      lcd.write((uint8_t)0);
      lcd.print("C   "); // Espacios para borrar residuos previos

      // Fila 1: Alarma:SI o Alarma:NO
      lcd.setCursor(0, 1);
      lcd.print("Alarma:");
      lcd.print(alarmaActiva ? "SI" : "NO");
      lcd.print("     "); // Espacios para limpiar restos anteriores
    }
  }
}
