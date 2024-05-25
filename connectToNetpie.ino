#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

//#define DHTPIN 2
//#define DHTTYPE DHT22

//DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Peem";
const char* password = "y22151579";

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "3913b46a-61f9-4e1d-9f00-d1f800bfd879";
const char* mqtt_username = "AJ4n5f6eNGtuDUuRo1BgWDhFuRhczF2C";
const char* mqtt_password = "9YJw9WurKGuJYiEebckniLm2YH5tjTgT";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[100];
char waterLevel = '0';
String pumpState = "off";
unsigned long ti = 0;

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting NETPIE2020 connection...");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("NETPIE2020 connected");
      // Subscribe to the correct topics
      if (client.subscribe("@shadow/data/update")) {
        Serial.println("Subscribed to @shadow/data/update");
      } else {
        Serial.println("Failed to subscribe to @shadow/data/update");
      }
      if (client.subscribe("@msg/pumpState")) {
        Serial.println("Subscribed to @msg/pumpState");
      } else {
        Serial.println("Failed to subscribe to @msg/pumpState");
      }
    } else {
      Serial.print("Failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
      delay(3000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String mess;
  for (int i = 0; i < length; i++) {
    mess += (char)payload[i];
  }
  Serial.println(mess);
  getMsg(topic, mess);
}

void getMsg(String topic_, String message_) {
  Serial.print("Processing message for topic: ");
  Serial.println(topic_);
  if (String(topic_) == "@msg/pumpState") {
    pumpState = message_;
    Serial.print("Updated pumpState to: ");
    Serial.println(pumpState);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  pinMode(15, OUTPUT); // D8
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //float humid = dht.readHumidity();

  if (Serial.available() > 0) {
    waterLevel = Serial.read();
  }

  if (pumpState == "on") {
    digitalWrite(15, HIGH);
    //Serial.println("Pump is ON");
  } else {
    digitalWrite(15, LOW);
    //Serial.println("Pump is OFF");
  }

  unsigned long ct = millis();
  if (ct - ti > 4000) {
    String data = "{\"data\":{\"waterLevel\":" + String(waterLevel) + ",\"pumpState\":\"" + pumpState + "\"}}";
    Serial.print("Publishing data: ");
    Serial.println(data);
    data.toCharArray(msg, (data.length() + 1));
    
    // Debug print before publishing
    Serial.println("Attempting to publish...");

    // Check if publish was successful
    if (client.publish("@shadow/data/update", msg)) {
      Serial.println("Publish successful");
    } else {
      Serial.println("Publish failed");
    }

    // Debug print after publishing
    Serial.println("Publish attempt completed.");
    
    ti = millis();
  }
}
