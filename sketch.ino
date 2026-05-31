/*
  ====================================================
  Simulador de Controle de Satelite
  Autor: Guilherme Kooji

  Pinos:
    A0  -> Potenciometro de TEMPERATURA (0 a 99 C)
    A1  -> Potenciometro de BATERIA (0 a 100%)
    A2  -> Potenciometro de ALTITUDE (400 a 800 km)
    2   -> Buzzer
    8   -> LED Verde   (NORMAL)
    9   -> LED Amarelo (ALERTA)
    10  -> LED Vermelho(CRITICO)
    A4  -> LCD SDA (I2C)
    A5  -> LCD SCL (I2C)

  Logica de estado (pior estado vence):
    TEMPERATURA:
      NORMAL  -> temp < 30 C
      ALERTA   -> 30 <= temp < 45 C
      CRITICO  -> temp >= 45 C

    BATERIA:
      NORMAL  -> bat >= 30%
      ALERTA   -> 15% <= bat < 30%
      CRITICO  -> bat < 15%

  Buzzer:
    ALERTA  -> 1 bip curto por ciclo
    CRITICO -> 3 bips rapidos por ciclo
  ====================================================
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define PIN_POT_TEMP  A0
#define PIN_POT_BAT   A1
#define PIN_POT_ALT   A2
#define PIN_BUZZER    2
#define PIN_LED_V     8
#define PIN_LED_A     9
#define PIN_LED_R     10

// ====================================================
void setup() {
  pinMode(PIN_LED_V,  OUTPUT);
  pinMode(PIN_LED_A,  OUTPUT);
  pinMode(PIN_LED_R,  OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();

  // Tela de boot
  lcd.setCursor(0, 0);
  lcd.print("SAT-CONTROL v3.0");
  lcd.setCursor(0, 1);
  lcd.print("  Testando...   ");

  // Pisca LEDs em sequencia no boot
  digitalWrite(PIN_LED_V, HIGH); delay(400); digitalWrite(PIN_LED_V, LOW);
  digitalWrite(PIN_LED_A, HIGH); delay(400); digitalWrite(PIN_LED_A, LOW);
  digitalWrite(PIN_LED_R, HIGH); delay(400); digitalWrite(PIN_LED_R, LOW);

  // Bip de boot
  tone(PIN_BUZZER, 1000); delay(200); noTone(PIN_BUZZER);

  lcd.clear();
}

// ====================================================
void loop() {
  // --- Leitura dos potenciometros ---
  int temp     = map(analogRead(PIN_POT_TEMP), 0, 1023, 0, 99);
  int bateria  = map(analogRead(PIN_POT_BAT),  0, 1023, 0, 100);
  int altitude = map(analogRead(PIN_POT_ALT),  0, 1023, 400, 800);

  // --- Estado da TEMPERATURA ---
  // 0=NORMAL 1=ALERTA 2=CRITICO
  int estadoTemp;
  if      (temp < 30) estadoTemp = 0;
  else if (temp < 45) estadoTemp = 1;
  else                estadoTemp = 2;

  // --- Estado da BATERIA ---
  int estadoBat;
  if      (bateria >= 30) estadoBat = 0;
  else if (bateria >= 15) estadoBat = 1;
  else                    estadoBat = 2;

  // --- Estado final: o pior dos dois vence ---
  int estado = max(estadoTemp, estadoBat);

  // --- LEDs ---
  digitalWrite(PIN_LED_V, estado == 0 ? HIGH : LOW);
  digitalWrite(PIN_LED_A, estado == 1 ? HIGH : LOW);
  digitalWrite(PIN_LED_R, estado == 2 ? HIGH : LOW);

  // --- Buzzer ---
  if (estado == 1) {
    tone(PIN_BUZZER, 1000); delay(150); noTone(PIN_BUZZER);
  } else if (estado == 2) {
    for (int i = 0; i < 3; i++) {
      tone(PIN_BUZZER, 2500); delay(80); noTone(PIN_BUZZER); delay(80);
    }
  } else {
    noTone(PIN_BUZZER);
  }

  // --- LCD Linha 0: Temperatura e Status ---
  lcd.setCursor(0, 0);
  lcd.print("T:");
  if (temp < 10) lcd.print(" ");
  lcd.print(temp);
  lcd.print("C ");
  if      (estado == 0) lcd.print("NORMAL ");
  else if (estado == 1) lcd.print("ALERTA  ");
  else                  lcd.print("CRITICO!");

  // --- LCD Linha 1: Bateria e Altitude ---
  lcd.setCursor(0, 1);
  lcd.print("BAT:");
  if (bateria < 100) lcd.print(" ");
  if (bateria < 10)  lcd.print(" ");
  lcd.print(bateria);
  lcd.print("% ");
  lcd.print(altitude);
  lcd.print("km  ");

  delay(800);
}
