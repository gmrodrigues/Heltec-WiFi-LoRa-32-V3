#include "Arduino.h"
#include "LoRaWan_APP.h"
#include "HT_SSD1306Wire.h"

#define RF_FREQUENCY          915000000  // Brasil
#define RX_TIMEOUT_VALUE      0          // RX contínuo
#define LORA_BANDWIDTH        0
#define LORA_SPREADING_FACTOR 7
#define LORA_CODINGRATE       1
#define LORA_PREAMBLE_LENGTH  8
#define LORA_IQ_INVERSION_ON  false

SSD1306Wire oled(0x3C, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

static RadioEvents_t RadioEvents;

char rxbuf[128];
volatile bool gotPacket = false;
int16_t lastRssi = 0;
int8_t lastSnr = 0;

void VextON() {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void show(const String& a, const String& b = "", const String& c = "", const String& d = "") {
  oled.clear();
  oled.setTextAlignment(TEXT_ALIGN_LEFT);
  oled.setFont(ArialMT_Plain_10);
  oled.drawString(0, 0, a);
  oled.drawString(0, 12, b);
  oled.drawString(0, 24, c);
  oled.drawString(0, 36, d);
  oled.display();
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  size = min(size, (uint16_t)(sizeof(rxbuf) - 1));
  memcpy(rxbuf, payload, size);
  rxbuf[size] = '\0';

  lastRssi = rssi;
  lastSnr = snr;
  gotPacket = true;

  Radio.Rx(RX_TIMEOUT_VALUE); // continua ouvindo
}

void OnRxTimeout(void) {
  Radio.Rx(RX_TIMEOUT_VALUE);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  VextON();
  delay(50);
  oled.init();
  show("LoRa RX", "915 MHz (BR)", "Init...");

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.RxDone = OnRxDone;
  RadioEvents.RxTimeout = OnRxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                    LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                    0, false, 0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  Radio.Rx(RX_TIMEOUT_VALUE);

  show("LoRa RX", "Pronto", "Aguardando...");
  Serial.println("RX pronto.");
}

void loop() {
  Radio.IrqProcess();

  if (gotPacket) {
    gotPacket = false;

    Serial.printf("RX <- %s | RSSI=%d dBm | SNR=%d dB\n", rxbuf, lastRssi, lastSnr);

    show("RX", String(rxbuf),
         "RSSI: " + String(lastRssi) + " dBm",
         "SNR: " + String(lastSnr) + " dB");
  }
}
