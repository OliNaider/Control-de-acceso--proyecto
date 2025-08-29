#include <WiFi.h>
#include <esp_now.h>

// Estructura del mensaje
typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message incomingData;
String datosRecibidos = "";

// Callback al recibir datos (IDF v5)
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  Serial.println(incomingData.msg);
  datosRecibidos = incomingData.msg;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }
  // Registrar callback de recepción
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  if(datosRecibidos == "FOTO") {
    Serial.println("fotito");
    delay(2000);
  }
}

