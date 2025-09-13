// Define el pin del ESP32 conectado al relé
const int relayPin = 4;

void setup() {
  // Configura el pin del relé como salida
  pinMode(relayPin, OUTPUT);
  // Inicialmente desactiva el relé para que la cerradura esté cerrada
  digitalWrite(relayPin, HIGH); // Algunos relés se activan con LOW, probá ambos si no funciona
}

void loop() {
  // Abre la cerradura (activa el relé)
  digitalWrite(relayPin, LOW); // O HIGH, depende del relé
  delay(5000); // Espera 5 segundos

  // Cierra la cerradura (desactiva el relé)
  digitalWrite(relayPin, HIGH); // O LOW, depende del relé
  delay(5000); // Espera 5 segundos
}