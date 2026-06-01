[README.md](https://github.com/user-attachments/files/28483087/README.md)
# 🛰️ SAT-CONTROL v4.0 — Simulador de Controle de Satélite

> Sistema embarcado em Arduino UNO que simula o painel de monitoramento de um satélite artificial em órbita, com registro de eventos em tempo real, histórico de alertas e sumários periódicos.

---

## 📋 Descrição do Projeto

O **SAT-CONTROL v4.0** lê continuamente três grandezas físicas — temperatura interna, nível de bateria e altitude orbital — e classifica o estado operacional do satélite em três níveis: **NORMAL**, **ALERTA** e **CRÍTICO**.

Toda transição de estado é registrada com timestamp preciso fornecido pelo módulo RTC DS1307, exibida no display LCD 16×2 e transmitida ao Serial Monitor com histórico completo dos últimos eventos e sumários a cada minuto.

---

## 🎯 Objetivo da Solução

- Simular um sistema de telemetria de satélite com sensores analógicos
- Classificar automaticamente o estado operacional com lógica de prioridade (**pior estado vence**)
- Registrar e exibir histórico de eventos e sumários periódicos com data e hora
- Emitir alertas visuais (LEDs) e sonoros (buzzer) conforme o nível de criticidade
- Demonstrar conceitos de sistemas embarcados: I2C, RTC, PWM, ADC e comunicação serial

---

## 🧰 Componentes Utilizados

| Componente | Qtd | Função |
|---|:---:|---|
| Arduino UNO | 1 | Microcontrolador principal |
| LCD 16×2 I2C (0x27) | 1 | Exibe temperatura, bateria, altitude e estado |
| Módulo RTC DS1307 | 1 | Relógio de tempo real para timestamps |
| Potenciômetro | 3 | Simulação de temperatura, bateria e altitude |
| LED Verde | 1 | Indicador de estado NORMAL |
| LED Amarelo | 1 | Indicador de estado ALERTA |
| LED Vermelho | 1 | Indicador de estado CRÍTICO |
| Resistor 220Ω | 3 | Proteção dos LEDs |
| Buzzer | 1 | Alarme sonoro em ALERTA e CRÍTICO |

---

## ⚙️ Funcionamento

### Leitura dos Sensores

Os três potenciômetros conectados às portas analógicas A0, A1 e A2 são lidos a cada ciclo. Os valores ADC (0–1023) são convertidos para faixas reais:

| Sensor | Pino | Faixa |
|---|:---:|---|
| Temperatura | A0 | 0 – 99 °C |
| Bateria | A1 | 0 – 100 % |
| Altitude | A2 | 400 – 800 km |

### Lógica de Estado

Cada sensor produz um nível individual. O estado global é o **pior entre os dois** (`estado = max(eTemp, eBat)`):

| Sensor | 🟢 NORMAL | 🟡 ALERTA | 🔴 CRÍTICO |
|---|---|---|---|
| Temperatura | < 30 °C | 30 – 44 °C | ≥ 45 °C |
| Bateria | ≥ 30 % | 15 – 29 % | < 15 % |

### Indicadores

| Estado | LED | Buzzer |
|---|---|---|
| NORMAL | 🟢 Verde | Silencioso |
| ALERTA | 🟡 Amarelo | 1 bip (1000 Hz) |
| CRÍTICO | 🔴 Vermelho | 3 bips rápidos (2500 Hz) |

### Display LCD

```
T: 38C  ALERTA  
BAT:  45%  612km
```

### Serial Monitor

A cada transição de estado:
```
=================================
 [EVENTO] 01/06/2026 08:03:42
 Transicao : NORMAL -> ALERTA
 Sensores  : T=38C  B=45%  A=612km
 Causa     : TEMPERATURA
 Tot Alertas : 1   Tot Criticos: 0
=================================
   HISTORICO DE EVENTOS (ultimos 10)
=================================
 #1  01/06/2026 08:03:42
      NORMAL -> ALERTA
      T=38C  B=45%  A=612km
      Causa: TEMP
=================================
```

A cada 1 minuto, um **sumário** é exibido com contadores do período e histórico dos últimos 5 sumários.

---

## 🔌 Estrutura do Circuito

### Barramento I2C (compartilhado)

| Componente | VCC | GND | SDA | SCL |
|---|:---:|:---:|:---:|:---:|
| LCD I2C (0x27) | 5V | GND | A4 | A5 |
| RTC DS1307 | 5V | GND | A4 | A5 |

### Mapeamento de Pinos

| Pino | Componente | Função |
|:---:|---|---|
| A0 | Potenciômetro TEMP | Leitura analógica de temperatura |
| A1 | Potenciômetro BAT | Leitura analógica de bateria |
| A2 | Potenciômetro ALT | Leitura analógica de altitude |
| A4 | SDA — LCD + RTC | Dados do barramento I2C |
| A5 | SCL — LCD + RTC | Clock do barramento I2C |
| D2 | Buzzer | Saída de alarme sonoro |
| D8 | LED Verde | Estado NORMAL |
| D9 | LED Amarelo | Estado ALERTA |
| D10 | LED Vermelho | Estado CRÍTICO |

---

## 🚀 Instruções de Execução

### Pré-requisitos

- Conta no [Wokwi](https://wokwi.com) (gratuita)
- Nenhuma instalação necessária — tudo roda no navegador

### Bibliotecas

Adicionar via **Library Manager** no Wokwi:

```
LiquidCrystal I2C  —  Frank de Brabander
RTClib             —  Adafruit
```

### Passo a Passo

1. Acesse [wokwi.com](https://wokwi.com) e crie um novo projeto **Arduino UNO**
2. Substitua o `sketch.ino` pelo arquivo `sat_control_v4_wokwi.ino`
3. Substitua o `diagram.json` pelo arquivo `diagram.json` deste repositório
4. Adicione as bibliotecas no Library Manager
5. Pressione ▶ **Play** para iniciar a simulação
6. Gire os potenciômetros para simular variações de temperatura e bateria
7. Clique na placa Arduino UNO para abrir o **Serial Monitor**

### Como Gerar Alertas

| Ação | Resultado |
|---|---|
| Pot TEMP > 30 °C | ALERTA de temperatura |
| Pot TEMP > 45 °C | CRÍTICO de temperatura |
| Pot BAT < 30 % | ALERTA de bateria |
| Pot BAT < 15 % | CRÍTICO de bateria |

> O sumário aparece automaticamente no Serial Monitor a cada **1 minuto** simulado.

---

## 📁 Arquivos do Projeto

```
📦 sat-control-v4/
 ┣ 📄 sat_control_v4_wokwi.ino   ← Código-fonte principal (C++ / Arduino)
 ┣ 📄 diagram.json               ← Esquema do circuito para o Wokwi
 ┗ 📄 README.md                  ← Este arquivo
```

---

## 👤 Integrante

| Nome | Função |
|---|---|
| **Guilherme Kooji Kubota** | Desenvolvimento completo do projeto |

---

<p align="center">
  <sub>SAT-CONTROL v4.0 • Guilherme Kooji Kubota • 2026</sub>
</p>
