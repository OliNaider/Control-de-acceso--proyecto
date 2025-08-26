#include <WiFi.h>
#include <esp_now.h>

// Estructura del mensaje
typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message incomingData;

// Callback actualizado para IDF v5
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));

  Serial.print("Mensaje recibido de: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->src_addr[i]);
    if (i < 5) Serial.print(":");
  }

  Serial.print(" → ");
  Serial.println(incomingData.msg);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }

  // Registra el callback
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // No hace falta nada en el loop
}

