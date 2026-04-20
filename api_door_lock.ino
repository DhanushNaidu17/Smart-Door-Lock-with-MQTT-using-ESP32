#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// ===== WIFI =====
const char* ssid = "Galaxy F415846";
const char* password = "12tree45";

// ===== MQTT =====
const char* mqtt_server = "a863e6bdc19548d1b79e4ea703dc0872.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "esp32door";
const char* mqtt_pass = "Door1234";

WiFiClientSecure espClient;
PubSubClient client(espClient);

// ===== PINS =====
#define RELAY_PIN 5
#define LED_PIN 4

bool doorState = false;

// ===== Publish Door Status =====
void publishStatus() {
  if (doorState) {
    client.publish("door/status", "OPEN");
    Serial.println("Status Sent: OPEN");
  } else {
    client.publish("door/status", "CLOSED");
    Serial.println("Status Sent: CLOSED");
  }
}

// ===== MQTT Callback =====
void callback(char* topic, byte* payload, unsigned int length) {

  String message;

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message received: ");
  Serial.println(message);

  if (message == "1") {
    digitalWrite(RELAY_PIN, HIGH);   // Door Open
    digitalWrite(LED_PIN, HIGH);     // LED ON
    doorState = true;
    Serial.println("Door Opened");
    publishStatus();
  }

  if (message == "0") {
    digitalWrite(RELAY_PIN, LOW);    // Door Close
    digitalWrite(LED_PIN, LOW);      // LED OFF
    doorState = false;
    Serial.println("Door Closed");
    publishStatus();
  }
}

// ===== MQTT Connection =====
void connectMQTT() {

  while (!client.connected()) {

    Serial.print("Connecting to MQTT... ");

    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("Connected");
      client.subscribe("door/control");
      publishStatus();
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying...");
      delay(2000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 Booting...");

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // Connect WiFi
  Serial.print("Connecting to WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();
}
