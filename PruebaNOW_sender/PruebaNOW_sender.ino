#include <WiFi.h>
#include <esp_now.h>


// Dirección MAC del receptor (ESP32-CAM)
uint8_t broadcastAddress[] = {0xB0, 0xA7, 0x32, 0xF1, 0xD7, 0xA4};

// Estructura del mensaje
typedef struct estructura {
  char msg[32];
} estructura;
estructura myData;

String datos = ""; 
int estado = 0;
void mensaje1();

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_now_init();
  registrarPeer();
  Serial.println(myData.msg);
}

void loop() {

  if(Serial.available() > 0){
    datos = Serial.readString();
    datos.trim();
    Serial.println(datos);
  }

  if(datos == "mandar"){
    estado = 1;
    Serial.println(estado);
  }

  if(estado == 1){
    mensaje1();
    datos = "";
    estado = 0;
  }
  
}

void registrarPeer() {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void mensaje1() {
  strcpy(myData.msg, "MAIL");
  esp_err_t result1 = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

}

