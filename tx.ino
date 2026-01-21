#include "Arduino.h"
#include "LoRaWan_APP.h"
#include "HT_SSD1306Wire.h"

#define RF_FREQUENCY          915000000  // Brasil
#define TX_OUTPUT_POWER       14         // dBm
#define LORA_BANDWIDTH        0          // 125 kHz
#define LORA_SPREADING_FACTOR 7          // SF7
#define LORA_CODINGRATE       1          // 4/5
#define LORA_PREAMBLE_LENGTH  8
#define LORA_IQ_INVERSION_ON  false

SSD1306Wire oled(0x3C, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

static RadioEvents_t RadioEvents;
volatile bool txDone = true;
uint32_t counter = 0;

void VextON() {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW); // Heltec: LOW = ON
}

void OnTxDone(void) {
  txDone = true;
}

void OnTxTimeout(void) {
  txDone = true;
}

void show(const String& a, const String& b = "", const String& c = "") {
  oled.clear();
  oled.setTextAlignment(TEXT_ALIGN_LEFT);
  oled.setFont(ArialMT_Plain_10);
  oled.drawString(0, 0, a);
  oled.drawString(0, 12, b);
  oled.drawString(0, 24, c);
  oled.display();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  VextON();
  delay(50);
  oled.init();
  show("LoRa TX", "915 MHz (BR)", "Init...");

  // init MCU + Radio
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;

  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);

  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, false,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

  show("LoRa TX", "Pronto", "Enviando...");
  Serial.println("TX pronto.");
}

void loop() {
  Radio.IrqProcess();

  if (txDone) {
    txDone = false;

    String msg = "HELLO " + String(counter++);
    Serial.println("TX -> " + msg);

    show("TX", msg, "915MHz SF7 BW125");
    Radio.Send((uint8_t*)msg.c_str(), msg.length());

    delay(1000);
  }
}
