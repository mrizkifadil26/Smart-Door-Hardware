#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <SPI.h>
#include <MFRC522.h>

const int     RST_PIN         = D1;
const int     SS_PIN          = D2;

const char*   WIFI_SSID       = "iPhone";
const char*   WIFI_PASSWORD   = "rajapertama";

const String  SERVER_HOST     = "192.168.43.69";
const int     SERVER_PORT     = 8000;

String        server          = "http://" + SERVER_HOST + ":" + SERVER_PORT + "/api/attendances";

String        uid             = "";
String        mac_address     = "";

String        json_request    = "";

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  
  Serial.begin(9600);
  while (!Serial) return;

  SPI.begin();
  mfrc522.PCD_Init();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
}

void loop() {
  
  String response;
  
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  mac_address   = getMacAddress();
  uid           = getUID();
  
  json_request  = json_encode(uid, mac_address);
  response      = send_data_to_server(json_request);
  send_data_to_arduino(response);
  delay(3000);
}

String send_data_to_server(String data) {
  HTTPClient http;
  
  http.begin(server);
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(data);
  String payload = http.getString();
  
  http.end();

  return payload;
}

String json_encode(String uid, String macAddress) {
  const int capacity    = JSON_OBJECT_SIZE(2) + 78;
  
  String pretty_json;
  DynamicJsonDocument json(capacity);
  
  json["uid"]           = uid;
  json["mac_address"]   = macAddress;
  
  serializeJsonPretty(json, pretty_json);
  
  return pretty_json;
}

String getMacAddress() {
  return WiFi.macAddress();
}

String getUID() {
  String content = "";
  
  for (int i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
    content.concat(i != 3 ? ":" : "");
  }
  
  content.toUpperCase();  
  mfrc522.PICC_HaltA();
  
  return content;
}

void send_data_to_arduino(String json) {
   Serial.println(json);
}
