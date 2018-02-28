#include <DHT_U.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "LivingRoom.h"
//#include "BedRoom.h"

#include <ESP8266WiFi.h>

#include <PubSubClient.h>
#include <DHT.h>

#define DHTTYPE DHT22
#define DHTPIN  D5

#define UP_PIN D8
#define DOWN_PIN D1
// Update these with values suitable for your network.

const char* ssid = "SSID";
const char* password = "password";
const char* mqtt_server = "192.168.1.13";

const int tempHumSendInterval = 30000; // Temp and Humidity sending interval

unsigned long tempHumLastSentMillis = 0;        // will store last temp was read

// Initialize DHT sensor 
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];


void setup_wifi() {

  delay(10);
  // We start by connectin to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  byte i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++i > 10) {
      Serial.println("Connection Failed! Rebooting...");
      delay(1000);
      ESP.restart();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_OTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Conver the incoming byte array to a string
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;
  Serial.println("Received target temp: " + message);

  setNextTargetTemp(message.toFloat());
}

void sendTemp() {
    float temp_c = dht.readTemperature();     // Read temperature as Celcius
    // Check if any reads failed and exit early (to try again).
    if (isnan(temp_c)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    temp_c = temp_c + TempAdjustment;
    dtostrf(temp_c , 2, 2, msg);
    Serial.print("Sending temperature:");
    Serial.println(temp_c);
    client.publish(TempOutTopic, msg);
}

void sendHum() {
    float humidity = dht.readHumidity();          // Read humidity (percent)
    if (isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    dtostrf(humidity , 2, 2, msg);
    //Serial.print("Sending humidity:");
    //Serial.println(humidity);
    client.publish(HumOutTopic, msg);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID)) {
      Serial.println("connected");
      // resubscribe
      client.subscribe(TargetTempInTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(UP_PIN, OUTPUT); 
  pinMode(DOWN_PIN, OUTPUT);

  pinMode(BUILTIN_LED, OUTPUT);  // set onboard LED as output
  digitalWrite(BUILTIN_LED, LOW);

  Serial.begin(115200);
  setup_wifi();
  setup_OTA();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();           // initialize temperature sensor


}


void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();

  ulong currentMillis = millis();

  // Temp and Humidity check interval
  if (currentMillis - tempHumLastSentMillis > tempHumSendInterval ) {
    tempHumLastSentMillis = millis();
    digitalWrite(BUILTIN_LED, LOW);
    sendTemp();
    sendHum();
    digitalWrite(BUILTIN_LED, HIGH);
  }

  // thermostat actions
  handleTSStage();
}


