#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>

PN532_I2C pn532_i2c(Wire);
PN532 nfc(pn532_i2c);

void setup() {
  Serial.begin(115200);

  // Iniciar I²C con los pines del ESP32
  Wire.begin(21, 22);
  Wire.setClock(100000); // Baja velocidad para evitar errores de NACK

  Serial.println("Iniciando PN532...");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("ERROR: No se detecta PN532. Revisar cableado y modo I2C.");
    while (1); // Se queda trabado si no detecta
  }

  Serial.print("PN532 detectado. Versión chip: 0x");
  Serial.println((versiondata >> 16) & 0xFF, HEX);

  nfc.SAMConfig(); // Inicializa para lectura de tarjetas
  Serial.println("Esperando tarjeta NFC...");
}

void loop() {
  uint8_t success;
  uint8_t uid[7];  // buffer para UID
  uint8_t uidLength;

  // Detectar tarjeta
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.print("Tarjeta detectada! UID: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(uid[i], HEX);
      if (i < uidLength - 1) Serial.print(" ");
    }
    Serial.println();
    delay(1000);
  }
}
