#include <WiFi.h>
#include <WiFiClient.h>
#include <WebSocketsClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <Ultrasonic.h>
#include <L298N.h>

#define RAINDROP_PIN 34
#define SOIL_MOISTURE_PIN 35
#define DHT_PIN 2
#define DHT_TYPE DHT11
#define ECHO_PIN 19
#define TRIG_PIN 18
#define ENB 32
#define IN3 16
#define IN4 17

const int tankHeight = 14;

const char* ssid = "Dialog 4G 523";
const char* password = "B47157D3";

const char* serverIP = "192.168.8.141";
const uint16_t serverPort = 8080;
const char* endPoint = "/GrowTrack/LoadData?type=esp";

WebSocketsClient socket = WebSocketsClient();

DHT dht = DHT(DHT_PIN, DHT_TYPE);
Ultrasonic ultrasonic = Ultrasonic(TRIG_PIN, ECHO_PIN);

L298N motor = L298N(ENB, IN3, IN4);

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("Client [%u] disconnected\n");
      break;
    case WStype_CONNECTED:
      Serial.printf("Client [%u] connected\n");
      break;
    case WStype_TEXT:
      Serial.printf("Received message: %s\n", payload);

      if (strcmp((char*)payload, "water") == 0) {
        int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
        int soilMoisturePercentage = map(soilMoistureValue, 0, 4095, 0, 100);

        int wateringDuration = calculateWateringTime(soilMoisturePercentage);

        Serial.printf("Activating water pump for %d milliseconds...\n", wateringDuration);
        motor.forward();
        delay(wateringDuration);
        motor.stop();
        socket.sendTXT("watered");
        Serial.println("Water pump deactivated");
      }
      break;
  }
}

int calculateWateringTime(int soilMoisture) {
  if (soilMoisture >= 70) {
    return 1000;
  } else if (soilMoisture >= 40 && soilMoisture < 70) {
    return 3000;
  } else {
    return 5000;
  }
}

String getSensorData() {
  int raindropValue = analogRead(RAINDROP_PIN);
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  int distance = ultrasonic.read();
  int waterLevel = tankHeight - distance;

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Faild to read DHT sensor");
    return "null";
  }

  StaticJsonDocument<200> doc;
  JsonObject data = doc.createNestedObject();
  data["raindrop"] = raindropValue;
  data["soilMoisture"] = soilMoistureValue;
  data["waterLevel"] = waterLevel;
  data["temperature"] = temperature;
  data["humidity"] = humidity;

  String stringJson;
  serializeJson(data, stringJson);

  return stringJson;
}

void setup() {
  Serial.begin(115200);

  pinMode(RAINDROP_PIN, INPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);

  dht.begin();

  motor.setSpeed(255);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conntecting...");
  }
  Serial.println("Conntected");

  socket.begin(serverIP, serverPort, endPoint);
  socket.onEvent(webSocketEvent);
  socket.setReconnectInterval(5000);
}

void loop() {
  socket.loop();

  static unsigned long lastPingTime = 0;
  if (millis() - lastPingTime > 5000) {
    String sensorData = getSensorData();
    socket.sendTXT("sensorData:" + sensorData);
    Serial.println("Sensor Data: " + sensorData);

    socket.sendPing();
    lastPingTime = millis();
  }
}
