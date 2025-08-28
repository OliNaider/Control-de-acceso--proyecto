#include <WiFi.h>
#include <esp_now.h>

// Dirección MAC del receptor (ESP32-CAM)
uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0xF1, 0xD7, 0xA4};

// Estructura del mensaje
typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message myData;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // Inicializar ESP-NOW
  esp_now_init();

  // Registrar el peer (receptor)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop() {
  // Preparar el mensaje
  strcpy(myData.msg, "Hola desde ESP32 🚀");
  esp_err_t result1 = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  delay(1000);
  strcpy(myData.msg, "BLABLABALBAL");
  esp_err_t result2 = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  //if (result == ESP_OK) {
   // Serial.println("Mensaje enviado...");
  //} //else {
    //Serial.println("Error al enviar ❌");
  //}

  delay(2000); // espera 2 segundos entre envíos
}

