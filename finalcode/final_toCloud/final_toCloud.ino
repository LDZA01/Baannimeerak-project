#include "DHT.h"
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <DHTStable.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define RXD2 16
#define TXD2 17
#define ESP_BAUD 115200

// Create an instance of the HardwareSerial class for Serial 2
HardwareSerial espSerial(2);

// Variables to store sensor data
float humidity = 0.0;
float temperature = 0.0;
int val_light = 0;
int val_sound = 0;
int val_rain = 0;

String incomingData = "";
String data_netpie = "" ;

// Network credentials and Netpie setup
const char* ssid = "Purem";
const char* password = "pure8888";
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "d38031c6-2cb0-4ced-9646-b03da1d63af0";
const char* mqtt_username = "1ZkE2P3W6DYMJkjrSSwykvtDNvdx7mMY";
const char* mqtt_password = "5kX6QKzRnruTueV1MBEVBmzPzCVSkuar";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[100];

// Timers
unsigned long previousMillisNetpie = 0;
unsigned long previousMillisGoogle = 0;
const unsigned long interval = 30000; // 30 seconds

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  String tpc;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
  tpc = getMsg(topic, message);
}

// Function prototypes

void sendToNetpie(String data_netpie);
void sendToGoogleSheets();
void processSensorData(String data);

void setup() {
  Serial.begin(115200);
  espSerial.begin(ESP_BAUD, SERIAL_8N1, RXD2, TXD2);
  
  Serial.println("Serial 2 started at 115200 baud rate");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  // Check for incoming data from Serial 2
  while (espSerial.available() > 0) {
    char espData = espSerial.read();
    Serial.print(espData);  //TESTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
    if (espData == '\n') {
      processSensorData(incomingData); // Parse JSON data
      data_netpie = incomingData ;
      incomingData = "";              // Clear buffer for next message
    } else {
      incomingData += espData;
    }
  }

  // Check timers
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisNetpie >= interval) {
    previousMillisNetpie = currentMillis;
    sendToNetpie(data_netpie);
  }

  if (currentMillis - previousMillisGoogle >= interval) {
    previousMillisGoogle = currentMillis;
    sendToGoogleSheets();
  }

  // Keep MQTT connection alive
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}


void sendToNetpie(String data_netpie) {
  if (!client.connected()) {
    reconnect();
  }
  data_netpie.toCharArray(msg , (data_netpie.length() + 1));
  client.publish("@shadow/data/update", msg);
  client.loop();
}

void sendToGoogleSheets() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://script.google.com/macros/s/AKfycby72I6Vx5Uu8oK4WchLVBtL3eK7fZPimqjVM6AXGKRoREFN7Q9SWERjmob0HipjmrHpMw/exec?sensor=Test&val_humidity="+String(humidity) 
    +"&val_temp="+String(temperature)
    +"&val_light="+String(val_light)
    +"&val_rain="+String(val_rain);
    http.begin(url.c_str());
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Google Sheets Response: " + http.getString());
    } else {
      Serial.println("Error sending data to Google Sheets: " + http.errorToString(httpCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot send data to Google Sheets.");
  }
}

void processSensorData(String data) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, data);

  if (error) {
    Serial.println("Failed to parse JSON: " + String(error.c_str()));
    return;
  }

  humidity = doc["data"]["Humidity"];
  temperature = doc["data"]["Temperature"];
  val_light = doc["data"]["LIGHT_Val"];
  val_sound = doc["data"]["Sound_Val"];
  val_rain = doc["data"]["Rain_val"];

  Serial.println("Parsed Data:");
  Serial.println("Humidity: " + String(humidity));
  Serial.println("Temperature: " + String(temperature));
  Serial.println("Light Value: " + String(val_light));
  Serial.println("Sound Value: " + String(val_sound));
  Serial.println("Rain Value: " + String(val_rain));
}

String getMsg(String topic_, String message_) {
}
