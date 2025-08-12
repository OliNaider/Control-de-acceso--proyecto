#include <WiFi.h>
#include <ESP_Mail_Client.h>

// WiFi
#define WIFI_SSID "carro2600"
#define WIFI_PASSWORD "colchones301"

// Nombre servidor smtp
#define SMTP_HOST "smtp.gmail.com"

/** Elegimos el puerto smtp 
 * 25  or esp_mail_smtp_port_25
 * 465 or esp_mail_smtp_port_465
 * 587 or esp_mail_smtp_port_587
 Para enviar un correo por Gmail a través del puerto 465 (SSL), la opción de
 aplicaciones poco seguras debe estar habilitada. https://myaccount.google.com/lesssecureapps?pli=1  */
#define SMTP_PORT esp_mail_smtp_port_465

// Credenciales de la cuenta remitente
//#define AUTHOR_EMAIL "tu_correo@gmail.com"
//#define AUTHOR_PASSWORD "XXXXXXXXX"
#define AUTHOR_EMAIL "IngresoSeguridadControl@gmail.com"
#define AUTHOR_PASSWORD "kvye pqwj sbkc xkpo"

// Declaramos una variable para referenciar la sesión SMTP 
SMTPSession smtp;

// Prototipo de la funcion callback para obtener el estado del envio del correo
void smtpCallback(SMTP_Status status);

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.print("Conectando al Acess point...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }

  Serial.println("");
  Serial.println("WiFi connectado.");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /** Habilitar debug via Serial port
   * none debug or 0
   * basic debug or 1
   * 
  */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declaramos una Sesion para poder configurar*/
  ESP_Mail_Session session;

  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;

  /* Declaramos una variable para el mensaje */
  SMTP_Message message;

  /* Especificar los encabezados*/
  message.sender.name = "ISC";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "Correo de Prueba";
  message.addRecipient("Oli Naider", "olinaider@gmail.com");

  String textMsg = "Hola, este el el cuerpo del correo electronico";
  message.text.content = textMsg.c_str();

  /** Set de caracteres para el texto:
   * us-ascii
   * utf-8
   * utf-7
   * The default value is utf-8
  */
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit; // sin codificación

  /** Prioridad del mensaje
   * esp_mail_smtp_priority_high or 1
   * esp_mail_smtp_priority_normal or 3
   * esp_mail_smtp_priority_low or 5
   * The default value is esp_mail_smtp_priority_low
  */
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;


  /* Conectar al servidor con la configuracios de la sesión*/
  if (!smtp.connect(&session))
    return;

  /* Envía el correo y cierra la sesión */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());

  // borrar los datos de los resultados el envio
  //smtp.sendingResult.clear();
  ESP_MAIL_PRINTF("Liberar memoria: %d\n", MailClient.getFreeHeap());
}

void loop()
{
}





void smtpCallback(SMTP_Status status)
{
  /* Imprime el estado actual*/
  Serial.println(status.info());

  /* Imprime los resultados del envio */
  if (status.success())
  {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Mensajes enviados con éxito: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Mensajes no enviados por falla: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Mensaje No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Estado: %s\n", result.completed ? "enviado" : "no enviado");
      ESP_MAIL_PRINTF("Fecha/Hora: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Destinatario: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Asunto: %s\n", result.subject);
    }
    Serial.println("----------------\n");

    //You need to clear sending result as the memory usage will grow up as it keeps the status, timstamp and
    //pointer to const char of recipients and subject that user assigned to the SMTP_Message object.

    //Because of pointer to const char that stores instead of dynamic string, the subject and recipients value can be
    //a garbage string (pointer points to undefind location) as SMTP_Message was declared as local variable or the value changed.

    //smtp.sendingResult.clear();
  }
}