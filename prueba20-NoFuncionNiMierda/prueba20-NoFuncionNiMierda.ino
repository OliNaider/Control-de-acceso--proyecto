//LIBRERIAS
#include "esp_camera.h"
#include <WiFi.h>
#include "board_config.h"
#include <esp_now.h>
#include <ESP_Mail_Client.h>

//WIFI
const char *ssid = "CARRO 2600 2.4";
const char *password = "colchones301";

//FUNCIONES
void startCameraServer();
void configuracionCamara();
void captureSendPhoto();
camera_config_t config;

//ESP-NOW 
typedef struct struct_message {
  char msg[32];
} struct_message;

struct_message incomingData;
String datosRecibidos = "";

//MAIL
#define emailSenderAccount    "IngresoSeguridadControl@gmail.com"
#define emailSenderPassword   "kvye pqwj sbkc xkpo"
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "ESP32-CAM Photo Captured"
#define emailRecipient        "olinaider@gmail.com"

//CALLBACK ESP-NOW
SMTPSession smtp;
void smtpCallback(SMTP_Status status);

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingDataBytes, int len) {
  memcpy(&incomingData, incomingDataBytes, sizeof(incomingData));
  Serial.print("ESP-NOW recibido: ");
  Serial.println(incomingData.msg);
  datosRecibidos = incomingData.msg;
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // WIFI
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  //CAMARA
  configuracionCamara();

  startCameraServer();
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  if (datosRecibidos == "FOTO") {
    Serial.println("Deteniendo streaming para sacar foto...");
    void stopCameraServer();   
    captureSendPhoto();  
    Serial.println("Foto enviada, retomando streaming...");
    startCameraServer();  
    datosRecibidos = "";
  }
}

//CONFIGURACION DE LA CAMARA
void configuracionCamara() {
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_VGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
  
  if(psramFound()){
    Serial.println("PSRAM disponible");
  } else {
    Serial.println("PSRAM NO disponible");
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

//SACAR FOTO
void captureSendPhoto() {
  // Reiniciar la cámara usando la config global
  esp_camera_deinit();
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Error re-iniciando la cámara");
    return;
  }

  // "Descartar" algunos frames para estabilizar la imagen
  camera_fb_t * fb = NULL;
  for (int i = 0; i < 2; i++) {
    fb = esp_camera_fb_get();
    if (fb) esp_camera_fb_return(fb);
  }

  // Captura final
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  // Crear el mensaje
  SMTP_Message message;
  message.enable.chunking = true;
  message.sender.name = "ESP32-CAM";
  message.sender.email = emailSenderAccount;
  message.subject = emailSubject;
  message.addRecipient("Oli", emailRecipient);

  String htmlMsg = "<h2>Photo captured with ESP32-CAM and attached in this email.</h2>";
  message.html.content = htmlMsg.c_str();
  message.html.charSet = "utf-8";
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_qp;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  // ✅ Adjuntar la foto directamente desde memoria
  message.addAttachment(fb->buf, fb->len, "photo.jpg", "image/jpeg");

  // Enviar el correo
  smtp.debug(0);
  smtp.callback(smtpCallback);

  Session_Config session_config;
  session_config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  session_config.time.gmt_offset = -3 * 3600;
  session_config.time.day_light_offset = 0;

  session_config.server.host_name = smtpServer;
  session_config.server.port = smtpServerPort;
  session_config.login.email = emailSenderAccount;
  session_config.login.password = emailSenderPassword;
  session_config.login.user_domain = "";

  if (!smtp.connect(&session_config)) return;
  if (!MailClient.sendMail(&smtp, &message, true))
    Serial.println("Error sending Email, " + smtp.errorReason());

  // Liberar el buffer de la cámara
  esp_camera_fb_return(fb);
}

void smtpCallback(SMTP_Status status){
  Serial.println(status.info());
  if (status.success()) {
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failed: %d\n", status.failedCount());
    smtp.sendingResult.clear();
  }
}


