#include <WiFi.h>
#include <esp_now.h>

// Dirección MAC del receptor (ESP32-CAM)
uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0xF1, 0xD7, 0xA4};

// Estructura del mensaje
typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message myData;

// Callback de confirmación de envío (IDF v5 usa des_addr)
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Estado del envío a: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->des_addr[i]);
    if (i < 5) Serial.print(":");
  }

  Serial.print(" → ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Éxito ✅" : "Fallo ❌");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }

  // Registrar callback de envío
  esp_now_register_send_cb(OnDataSent);

  // Registrar el peer (receptor)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error agregando peer");
    return;
  }
}

void loop() {
  // Preparar el mensaje
  strcpy(myData.msg, "Hola desde ESP32 🚀");
  delay(1000);
  strcpy(myData.msg, "BLABLABALBAL");
  // Enviar mensaje
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Mensaje enviado...");
  } else {
    Serial.println("Error al enviar ❌");
  }

  delay(2000); // espera 2 segundos entre envíos
}

