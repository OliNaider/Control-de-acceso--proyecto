#include <WiFi.h>
#include <esp_now.h>


// Dirección MAC del receptor (ESP32-CAM)
uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0xF1, 0xD7, 0xA4};

// Estructura del mensaje
typedef struct estructura {
  char msg[32];
} estructura;
estructura myData;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // Inicializar ESP-NOW
  esp_now_init();
  registrarPeer();
  mensaje1();
}

void loop() {

  return;
  
}

void registrarPeer() {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void mensaje1() {
  strcpy(myData.msg, "FOTO");
  esp_err_t result1 = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

}

