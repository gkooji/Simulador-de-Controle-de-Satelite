/* ====================================================
   SAT-CONTROL v4.0 — Simulador de Controle de Satelite
   Autor : Guilherme Kooji

   BIBLIOTECAS:
     • LiquidCrystal I2C  — Frank de Brabander
     • RTClib              — Adafruit

   PINOS:
     A0  → Potenciometro TEMPERATURA  (0–99 C)
     A1  → Potenciometro BATERIA      (0–100 %)
     A2  → Potenciometro ALTITUDE     (400–800 km)
     2   → Buzzer
     8   → LED Verde    (NORMAL)
     9   → LED Amarelo  (ALERTA)
     10  → LED Vermelho (CRITICO)
     A4  → SDA — LCD I2C + RTC
     A5  → SCL — LCD I2C + RTC
==================================================== */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

// ─── Hardware ─────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307        rtc;

// ─── Pinos ────────────────────────────────────────
#define PIN_POT_TEMP  A0
#define PIN_POT_BAT   A1
#define PIN_POT_ALT   A2
#define PIN_BUZZER     2
#define PIN_LED_VERDE  8
#define PIN_LED_AMAR   9
#define PIN_LED_VERM  10

// ─── Limites ──────────────────────────────────────
#define TEMP_ALERTA   30
#define TEMP_CRITICO  45
#define BAT_ALERTA    30
#define BAT_CRITICO   15
#define FREQ_ALERTA   1000
#define FREQ_CRITICO  2500
#define INTERVALO_SUMARIO_MS 60000UL

// ─── Historico de eventos (ultimos 10) ────────────
#define MAX_HIST 10

struct Evento {
  char timestamp[20];
  int  de;
  int  para;
  int  temp;
  int  bat;
  int  alt;
  bool causaTemp;
  bool causaBat;
};

Evento historico[MAX_HIST];
int    histIdx   = 0;
int    histTotal = 0;

// ─── Historico de sumarios (ultimos 5) ────────────
#define MAX_SUM 5

struct Sumario {
  char timestamp[20];
  int  temp;
  int  bat;
  int  alt;
  int  estado;
  long alertas;
  long criticos;
  long causaTemp;
  long causaBat;
};

Sumario sumarios[MAX_SUM];
int     sumIdx   = 0;
int     sumTotal = 0;

// ─── Contadores ───────────────────────────────────
int  estadoAnterior    = 0;
long totalAlertas      = 0;
long totalCriticos     = 0;
long alertasNoMinuto   = 0;
long criticosNoMinuto  = 0;
long causaTempNoMinuto = 0;
long causaBatNoMinuto  = 0;
unsigned long ultimoSumario = 0;

// ─── Helpers ──────────────────────────────────────
const char* nomeEstado(int e) {
  if (e == 0) return "NORMAL";
  if (e == 1) return "ALERTA";
  return "CRITICO";
}

void formatDateTime(const DateTime& dt, char* buf) {
  sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d",
    dt.day(), dt.month(), dt.year(),
    dt.hour(), dt.minute(), dt.second());
}

void printSep()  { Serial.println(F("=================================")); }
void printSep2() { Serial.println(F("---------------------------------")); }

// ─── Grava evento no historico circular ───────────
void gravarHistorico(int de, int para, int temp, int bat, int alt,
                     bool cTemp, bool cBat) {
  DateTime dt = rtc.now();
  formatDateTime(dt, historico[histIdx].timestamp);
  historico[histIdx].de        = de;
  historico[histIdx].para      = para;
  historico[histIdx].temp      = temp;
  historico[histIdx].bat       = bat;
  historico[histIdx].alt       = alt;
  historico[histIdx].causaTemp = cTemp;
  historico[histIdx].causaBat  = cBat;
  histIdx = (histIdx + 1) % MAX_HIST;
  if (histTotal < MAX_HIST) histTotal++;
}

// ─── Grava sumario no historico circular ──────────
void gravarSumario(int temp, int bat, int alt, int estado) {
  DateTime dt = rtc.now();
  formatDateTime(dt, sumarios[sumIdx].timestamp);
  sumarios[sumIdx].temp      = temp;
  sumarios[sumIdx].bat       = bat;
  sumarios[sumIdx].alt       = alt;
  sumarios[sumIdx].estado    = estado;
  sumarios[sumIdx].alertas   = alertasNoMinuto;
  sumarios[sumIdx].criticos  = criticosNoMinuto;
  sumarios[sumIdx].causaTemp = causaTempNoMinuto;
  sumarios[sumIdx].causaBat  = causaBatNoMinuto;
  sumIdx = (sumIdx + 1) % MAX_SUM;
  if (sumTotal < MAX_SUM) sumTotal++;
}

// ─── Imprime historico de eventos ─────────────────
void imprimirHistorico() {
  printSep();
  Serial.println(F("   HISTORICO DE EVENTOS (ultimos 10)"));
  printSep();
  if (histTotal == 0) {
    Serial.println(F(" Nenhum evento registrado ainda."));
    printSep();
    return;
  }
  int inicio = (histTotal < MAX_HIST) ? 0 : histIdx;
  for (int i = 0; i < histTotal; i++) {
    int idx = (inicio + i) % MAX_HIST;
    Evento& e = historico[idx];
    Serial.print(F(" #")); Serial.print(i + 1); Serial.print(F("  "));
    Serial.println(e.timestamp);
    Serial.print(F("      ")); Serial.print(nomeEstado(e.de));
    Serial.print(F(" -> ")); Serial.println(nomeEstado(e.para));
    Serial.print(F("      T=")); Serial.print(e.temp); Serial.print(F("C"));
    Serial.print(F("  B=")); Serial.print(e.bat); Serial.print(F("%"));
    Serial.print(F("  A=")); Serial.print(e.alt); Serial.println(F("km"));
    Serial.print(F("      Causa:"));
    if (e.causaTemp) Serial.print(F(" TEMP"));
    if (e.causaBat)  Serial.print(F(" BAT"));
    if (!e.causaTemp && !e.causaBat) Serial.print(F(" -"));
    Serial.println();
    if (i < histTotal - 1) printSep2();
  }
  printSep();
}

// ─── Imprime historico de sumarios ────────────────
void imprimirHistoricoSumarios() {
  printSep();
  Serial.println(F("   HISTORICO DE SUMARIOS (ultimos 5)"));
  printSep();
  if (sumTotal == 0) {
    Serial.println(F(" Nenhum sumario registrado ainda."));
    printSep();
    return;
  }
  int inicio = (sumTotal < MAX_SUM) ? 0 : sumIdx;
  for (int i = 0; i < sumTotal; i++) {
    int idx = (inicio + i) % MAX_SUM;
    Sumario& s = sumarios[idx];
    Serial.print(F(" MINUTO #")); Serial.print(i + 1);
    Serial.print(F("  ")); Serial.println(s.timestamp);
    Serial.print(F("   Estado  : ")); Serial.println(nomeEstado(s.estado));
    Serial.print(F("   T=")); Serial.print(s.temp); Serial.print(F("C"));
    Serial.print(F("  B=")); Serial.print(s.bat); Serial.print(F("%"));
    Serial.print(F("  A=")); Serial.print(s.alt); Serial.println(F("km"));
    Serial.print(F("   Alertas : ")); Serial.print(s.alertas);
    Serial.print(F("  Criticos: ")); Serial.println(s.criticos);
    if (s.alertas > 0 || s.criticos > 0) {
      Serial.print(F("   Causas  :"));
      if (s.causaTemp > 0) { Serial.print(F(" TEMP x")); Serial.print(s.causaTemp); }
      if (s.causaBat  > 0) { Serial.print(F(" BAT x"));  Serial.print(s.causaBat);  }
      Serial.println();
    }
    if (i < sumTotal - 1) printSep2();
  }
  printSep();
}

// ─── Log de transicao ─────────────────────────────
void logTransicao(int anterior, int novo,
                  int temp, int bat, int alt,
                  int eTemp, int eBat) {
  bool cTemp = (eTemp >= 1);
  bool cBat  = (eBat  >= 1);

  gravarHistorico(anterior, novo, temp, bat, alt, cTemp, cBat);

  char ts[20];
  formatDateTime(rtc.now(), ts);
  Serial.println();
  printSep();
  Serial.print(F(" [EVENTO] ")); Serial.println(ts);
  Serial.print(F(" Transicao : "));
  Serial.print(nomeEstado(anterior)); Serial.print(F(" -> ")); Serial.println(nomeEstado(novo));
  Serial.print(F(" Sensores  : T=")); Serial.print(temp); Serial.print(F("C"));
  Serial.print(F("  B=")); Serial.print(bat); Serial.print(F("%"));
  Serial.print(F("  A=")); Serial.print(alt); Serial.println(F("km"));
  Serial.print(F(" Causa     :"));
  if (cTemp) Serial.print(F(" TEMPERATURA"));
  if (cBat)  Serial.print(F(" BATERIA"));
  Serial.println();
  Serial.print(F(" Tot Alertas : ")); Serial.print(totalAlertas);
  Serial.print(F("   Tot Criticos: ")); Serial.println(totalCriticos);
  printSep();

  imprimirHistorico();
}

// ─── Sumario do minuto ────────────────────────────
void logSumario(int temp, int bat, int alt, int estado) {
  // Grava antes de zerar contadores
  gravarSumario(temp, bat, alt, estado);

  char ts[20];
  formatDateTime(rtc.now(), ts);
  Serial.println();
  printSep();
  Serial.println(F("         SUMARIO DO MINUTO"));
  printSep();
  Serial.print(F(" Data/Hora     : ")); Serial.println(ts);
  Serial.print(F(" Estado atual  : ")); Serial.println(nomeEstado(estado));
  printSep2();
  Serial.print(F(" Temperatura   : ")); Serial.print(temp); Serial.println(F(" C"));
  Serial.print(F(" Bateria       : ")); Serial.print(bat);  Serial.println(F(" %"));
  Serial.print(F(" Altitude      : ")); Serial.print(alt);  Serial.println(F(" km"));
  printSep2();
  Serial.print(F(" Alertas       : ")); Serial.println(alertasNoMinuto);
  Serial.print(F(" Criticos      : ")); Serial.println(criticosNoMinuto);
  if (alertasNoMinuto > 0 || criticosNoMinuto > 0) {
    Serial.println(F(" Causas (ocorrencias):"));
    if (causaTempNoMinuto > 0) { Serial.print(F("   Temperatura : ")); Serial.println(causaTempNoMinuto); }
    if (causaBatNoMinuto  > 0) { Serial.print(F("   Bateria     : ")); Serial.println(causaBatNoMinuto);  }
  } else {
    Serial.println(F(" Causas        : Nenhuma"));
  }
  printSep2();
  Serial.print(F(" Tot Alertas   : ")); Serial.println(totalAlertas);
  Serial.print(F(" Tot Criticos  : ")); Serial.println(totalCriticos);
  printSep();

  // Historico de eventos e sumarios anteriores
  imprimirHistorico();
  imprimirHistoricoSumarios();

  // Zera contadores do minuto
  alertasNoMinuto   = 0;
  criticosNoMinuto  = 0;
  causaTempNoMinuto = 0;
  causaBatNoMinuto  = 0;
}

// ─── Setup ────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_AMAR,  OUTPUT);
  pinMode(PIN_LED_VERM,  OUTPUT);
  pinMode(PIN_BUZZER,    OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("SAT-CONTROL v4.0");
  lcd.setCursor(0, 1); lcd.print("  Iniciando...  ");

  // Teste sequencial de LEDs
  digitalWrite(PIN_LED_VERDE, HIGH); delay(300); digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_AMAR,  HIGH); delay(300); digitalWrite(PIN_LED_AMAR,  LOW);
  digitalWrite(PIN_LED_VERM,  HIGH); delay(300); digitalWrite(PIN_LED_VERM,  LOW);
  tone(PIN_BUZZER, 1000); delay(200); noTone(PIN_BUZZER);

  // ── RTC DS1307 ──────────────────────────────────
  rtc.begin();
  // Define hora de referencia fixa: 01/06/2025 08:00:00
  // Ajuste a data/hora conforme necessario
  rtc.adjust(DateTime(2026, 6, 1, 18, 44, 0));

  lcd.clear();

  char ts[20];
  formatDateTime(rtc.now(), ts);
  printSep();
  Serial.println(F("   SAT-CONTROL v4.0 — Wokwi"));
  Serial.print  (F("   Boot: ")); Serial.println(ts);
  printSep();
  Serial.println(F(" Gire os potenciometros para"));
  Serial.println(F(" gerar alertas e criticos."));
  Serial.println(F(" Sumario a cada 1 minuto."));
  printSep();

  ultimoSumario = millis();
}

// ─── Loop ─────────────────────────────────────────
void loop() {
  int temp     = map(analogRead(PIN_POT_TEMP), 0, 1023,   0,  99);
  int bateria  = map(analogRead(PIN_POT_BAT),  0, 1023,   0, 100);
  int altitude = map(analogRead(PIN_POT_ALT),  0, 1023, 400, 800);

  int eTemp;
  if      (temp < TEMP_ALERTA)  eTemp = 0;
  else if (temp < TEMP_CRITICO) eTemp = 1;
  else                          eTemp = 2;

  int eBat;
  if      (bateria >= BAT_ALERTA)  eBat = 0;
  else if (bateria >= BAT_CRITICO) eBat = 1;
  else                             eBat = 2;

  int estado = max(eTemp, eBat);

  // ── Transicao de estado ──────────────────────────
  if (estado != estadoAnterior) {
    if (estado >= 1) {
      if (estado == 1) { totalAlertas++;  alertasNoMinuto++;  }
      if (estado == 2) { totalCriticos++; criticosNoMinuto++; }
      if (eTemp >= 1) causaTempNoMinuto++;
      if (eBat  >= 1) causaBatNoMinuto++;
    }
    logTransicao(estadoAnterior, estado, temp, bateria, altitude, eTemp, eBat);
    estadoAnterior = estado;
  }

  // ── Sumario a cada 1 minuto ──────────────────────
  if (millis() - ultimoSumario >= INTERVALO_SUMARIO_MS) {
    logSumario(temp, bateria, altitude, estado);
    ultimoSumario = millis();
  }

  // ── LEDs ────────────────────────────────────────
  digitalWrite(PIN_LED_VERDE, estado == 0 ? HIGH : LOW);
  digitalWrite(PIN_LED_AMAR,  estado == 1 ? HIGH : LOW);
  digitalWrite(PIN_LED_VERM,  estado == 2 ? HIGH : LOW);

  // ── Buzzer ──────────────────────────────────────
  if (estado == 1) {
    tone(PIN_BUZZER, FREQ_ALERTA); delay(150); noTone(PIN_BUZZER);
  } else if (estado == 2) {
    for (int i = 0; i < 3; i++) {
      tone(PIN_BUZZER, FREQ_CRITICO); delay(80);
      noTone(PIN_BUZZER);             delay(80);
    }
  } else {
    noTone(PIN_BUZZER);
  }

  // ── LCD linha 0: temperatura + estado ───────────
  lcd.setCursor(0, 0);
  lcd.print("T:");
  if (temp < 10) lcd.print(" ");
  lcd.print(temp); lcd.print("C ");
  if      (estado == 0) lcd.print("NORMAL  ");
  else if (estado == 1) lcd.print("ALERTA  ");
  else                  lcd.print("CRITICO!");

  // ── LCD linha 1: bateria + altitude ─────────────
  lcd.setCursor(0, 1);
  lcd.print("BAT:");
  if (bateria < 100) lcd.print(" ");
  if (bateria <  10) lcd.print(" ");
  lcd.print(bateria); lcd.print("% ");
  lcd.print(altitude); lcd.print("km ");

  delay(800);
}
