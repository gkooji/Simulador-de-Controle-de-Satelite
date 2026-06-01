🛰️ Simulador de Controle de Satélite
Descrição
Sistema embarcado em Arduino Uno que simula o painel de monitoramento de um satélite em órbita baixa. Três potenciômetros controlam temperatura, bateria e altitude. O LCD exibe os dados em tempo real e LEDs + buzzer sinalizam o estado operacional.

Objetivo
Demonstrar como sistemas embarcados monitoram e sinalizam estados críticos em dispositivos espaciais, integrando entradas analógicas, display LCD, sinalização visual (LEDs) e sonora (buzzer).

Componentes
ComponenteQtdPinoArduino Uno1—LCD 16x2 I2C1SDA=A4, SCL=A5Potenciômetro (Temperatura)1A0Potenciômetro (Bateria)1A1Potenciômetro (Altitude)1A2Buzzer1D2LED Verde (NOMINAL)1D8LED Amarelo (ALERTA)1D9LED Vermelho (CRÍTICO)1D10Resistor 220Ω3—

Funcionamento
O sistema avalia temperatura e bateria separadamente e adota o pior estado entre os dois:
Temperatura
EstadoCondiçãoNOMINALtemp < 30°CALERTA30°C ≤ temp < 45°CCRÍTICOtemp ≥ 45°C
Bateria
EstadoCondiçãoNOMINALbateria ≥ 30%ALERTA15% ≤ bateria < 30%CRÍTICObateria < 15%
Sinalização
Estado finalLEDBuzzerNOMINALVerdeSilenciosoALERTAAmarelo1 bip por cicloCRÍTICOVermelho3 bips rápidos por ciclo
LCD — Linha 1: Temperatura + Status
LCD — Linha 2: Bateria (%) + Altitude (km)
No boot, os 3 LEDs piscam em sequência e o buzzer emite 1 bip confirmando o funcionamento.

Estrutura do Circuito
Arduino Uno
├── A0  → Potenciômetro Temperatura (SIG)
├── A1  → Potenciômetro Bateria (SIG)
├── A2  → Potenciômetro Altitude (SIG)
├── A4  → LCD SDA
├── A5  → LCD SCL
├── D2  → Buzzer (pino 1)
├── D8  → Resistor 220Ω → LED Verde   (cátodo → GND)
├── D9  → Resistor 220Ω → LED Amarelo (cátodo → GND)
└── D10 → Resistor 220Ω → LED Vermelho (cátodo → GND)

Potenciômetros: VCC → 5V | GND → GND | SIG → A0, A1 ou A2
LCD I2C:        VCC → 5V | GND → GND
Buzzer:         pino 1 → D2 | pino 2 → GND

Instruções de Execução
Wokwi

Acesse wokwi.com e crie um projeto Arduino Uno
Cole o conteúdo de satellite_control.ino no editor
Clique em diagram.json e substitua pelo arquivo deste repositório
Clique em ▶ Play
Gire os potenciômetros para simular os valores:

Esquerda → Temperatura (0–99°C)
Centro → Bateria (0–100%)
Direita → Altitude (400–800 km)




Link da simulação: (adicionar após criar no Wokwi)

Arduino IDE

Instale a biblioteca LiquidCrystal I2C pelo Gerenciador de Bibliotecas
Monte o circuito conforme o diagrama
Faça upload de satellite_control.ino


Estrutura do Repositório
satellite-control-simulator/
├── satellite_control.ino
├── diagram.json
└── README.md

Integrantes
Nome Completo: Guilherme Kooji
