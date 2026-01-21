# 📡 Heltec WiFi LoRa 32 V3 – Setup Arduino IDE (Brasil 915 MHz)

Este guia mostra **passo a passo** como configurar o Arduino IDE para rodar um **exemplo LoRa TX/RX mínimo com OLED funcionando**, usando a **Heltec WiFi LoRa 32 V3** (SX1262).

---

## 🧩 Hardware usado

* **Placa:** Heltec WiFi LoRa 32 V3
* **Chip LoRa:** SX1262
* **Display:** OLED 0.96" 128×64
* **Frequência Brasil:** **915 MHz**
* **Antena:** obrigatória (não ligue sem antena)

---

## 🛠️ 1. Instalar Arduino IDE

Recomendado:

* **Arduino IDE 2.x**

Baixe em:
[https://www.arduino.cc/en/software](https://www.arduino.cc/en/software)

---

## 🧠 2. Instalar suporte ESP32

### Arduino IDE → Preferences

Adicione no campo **Additional Boards Manager URLs**:

```text
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Clique em **OK**.

---

### Boards Manager

* Vá em **Tools → Board → Boards Manager**
* Procure por **esp32**
* Instale:

```
esp32 by Espressif Systems
```

Versão recomendada:

* ✅ **2.0.14** ou **2.0.17**

---

## 📦 3. Instalar bibliotecas corretas (CRÍTICO)

### ❌ NÃO misture bibliotecas

Use **APENAS** a biblioteca oficial da Heltec.

---

### Library Manager

Abra **Library Manager** e instale:

```
Heltec ESP32 Dev-Boards
```

Autor:

```
Heltec Automation
```

> ⚠️ NÃO instale:
>
> * Heltec_ESP32_LoRa_v3 (terceiros)
> * RadioLib separado
> * Outras libs OLED

Essa lib **já inclui**:

* SX1262 (Radio)
* OLED
* VEXT
* Pinagem correta da V3

---

## 🧩 4. Selecionar a board correta

### Tools → Board

Selecione:

```
Heltec WiFi LoRa 32 (V3)
```

### Configurações recomendadas

| Opção            | Valor   |
| ---------------- | ------- |
| USB CDC On Boot  | Enabled |
| Flash Frequency  | 80MHz   |
| Flash Mode       | QIO     |
| Partition Scheme | Default |
| Upload Speed     | 921600  |

---

## 🔌 5. Conectar a placa

* Conecte via **USB-C**
* Selecione a porta correta em **Tools → Port**
* Conecte **a antena LoRa** antes de ligar

---

## 📡 6. Exemplo mínimo – TX (915 MHz)

Crie um sketch chamado `tx.ino`:

```cpp
#include "LoRaWan_APP.h"
#include "HT_SSD1306Wire.h"

// Frequência Brasil
#define RF_FREQUENCY 915000000

SSD1306Wire display(0x3C, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

static RadioEvents_t RadioEvents;

void OnTxDone() {
  Serial.println("TX done");
}

void setup() {
  Serial.begin(115200);

  // Liga alimentação do OLED
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(100);

  display.init();
  display.clear();
  display.drawString(0, 0, "LoRa TX");
  display.drawString(0, 12, "915 MHz BR");
  display.display();

  // Inicializa placa Heltec
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.TxDone = OnTxDone;
  Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(
    MODEM_LORA,
    14,         // potência dBm
    0,
    0,          // 125 kHz
    7,          // SF7
    1,          // CR 4/5
    8,
    false,
    true,
    0,
    0,
    false,
    3000
  );
}

void loop() {
  const char *msg = "HELLO LORA";
  Radio.Send((uint8_t *)msg, strlen(msg));

  display.clear();
  display.drawString(0, 0, "TX:");
  display.drawString(0, 12, msg);
  display.display();

  delay(2000);
}
```

---

## 📥 7. Exemplo mínimo – RX

Crie um sketch chamado `rx.ino`:

```cpp
#include "LoRaWan_APP.h"
#include "HT_SSD1306Wire.h"

#define RF_FREQUENCY 915000000

SSD1306Wire display(0x3C, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

static RadioEvents_t RadioEvents;

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
  payload[size] = 0;

  Serial.printf("RX: %s RSSI=%d\n", payload, rssi);

  display.clear();
  display.drawString(0, 0, "RX:");
  display.drawString(0, 12, (char *)payload);
  display.drawString(0, 30, "RSSI:");
  display.drawString(40, 30, String(rssi));
  display.display();

  Radio.Rx(0);
}

void setup() {
  Serial.begin(115200);

  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(100);

  display.init();
  display.clear();
  display.drawString(0, 0, "LoRa RX");
  display.drawString(0, 12, "915 MHz BR");
  display.display();

  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);

  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetRxConfig(
    MODEM_LORA,
    0,
    7,
    1,
    0,
    8,
    0,
    false,
    0,
    true,
    0,
    0,
    false,
    true
  );

  Radio.Rx(0);
}

void loop() {
  Radio.IrqProcess();
}
```

---

## ✅ 8. Teste final

1. Grave **RX** em uma placa
2. Grave **TX** na outra
3. Ambas em **915 MHz**
4. OLED deve mostrar TX/RX
5. Serial Monitor confirma pacotes

---

## 🚨 Erros comuns (evite)

❌ Usar `LoRa.h`
❌ Misturar RadioLib
❌ Não ligar antena
❌ Frequência 868 no Brasil
❌ Criar outro `display` global (já existe na lib)

---

## 🧠 Conclusão

* A **Heltec V3 funciona**, mas **só com o stack correto**
* A lib oficial **já resolve VEXT, OLED, SX1262**