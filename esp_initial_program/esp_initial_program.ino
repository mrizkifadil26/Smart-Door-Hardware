#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <SPI.h>
#include <MFRC522.h>

const char* ssid = "iPhone";
const char* password = "rajaterakhir";

String server = "http://192.168.43.69:8000/api/attendances";

String uid = "00 00 00 00";
int classId = 1;
String macAddress = "";
String timestamps = "2020-04-19T00:30:07.000Z";

String jsonRequest;

const char* name;
const char* nim;
bool access = false;
bool isLecturer = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) return;

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:  
  macAddress = getMacAddress();
  jsonRequest = jsonEncode(uid, classId, macAddress, timestamps);

  String uid = getUID();
  if (uid.substring(1) == "25 26 60 2B") //change here the UID of the card/cards that you want to give access
  {
    if (WiFi.status() == WL_CONNECTED) {
      sendDataToServer(jsonRequest);
    } else {
      Serial.println("Error sending data");
    }
  } else {
    Serial.println("Access denied");
    Serial.println();
  }
  
//  if (WiFi.status() == WL_CONNECTED) {
//    sendDataToServer(jsonRequest);
//  } else {
//    Serial.println("Error sending data");
//  }

  delay(10000);
}

void sendDataToServer(String data) {
  HTTPClient http;
  http.begin("http://192.168.43.69:8000/api/attendances");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(data);
  String payload = http.getString();

  Serial.print("HTTP Code: ");
  Serial.println(httpCode);
  Serial.print("Payload: ");
  Serial.println(payload);
  
  http.end();
}

String jsonEncode(String uid, int classId, String macAddress, String timestamps) {
  String prettyJson;
  const int capacity = JSON_OBJECT_SIZE(4) + 91;
  DynamicJsonDocument json(capacity);
  json["uid"] = uid;
  json["class_id"] = classId;
  json["mac_address"] = macAddress;
  json["timestamps"] = timestamps;
  serializeJsonPretty(json, prettyJson);
  return prettyJson;
}

String jsonDecode(String data) {
  const size_t capacity = JSON_OBJECT_SIZE(4) + 70;
  StaticJsonDocument<capacity> json;
  deserializeJson(json, data);

  name = json["name"];
  nim = json["nim"];
  access = json["access"];
  isLecturer = json["is_lecturer"];
}

String getMacAddress() {
  return WiFi.macAddress();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authentified()) {
    String header = "HTTP/1.1 301 OK\r\nLocation: /login\r\nCache-Control: no-cache\r\n\r\n";
    server.sendContent(header);
    return;
  }
  String content = "<html><h2>WELCOME TO NODEMCU WEB SERVER!</font></h2><br>";
  content += "<br><a href=\"/login?DISCONNECT=YES\"><b>LOGOUT</b></font></button></font></body></html>";
  
  if (server.hasHeader("User-Agent")){
    content += "<p>The user agent used is : " + server.header("User-Agent") + "<br><br>";
  }
  server.send(200, "text/html", content);
}

String getUID() {
  String content = "";
  byte letter;

  Serial.print("UID: ");
  for (int i = 0; i < mfrc522.uid.size; i++) {
  //    readCard[i] = mfrc522.uid.uidByte[i];
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println(content);
  mfrc522.PICC_HaltA();
  
  return content;
}
