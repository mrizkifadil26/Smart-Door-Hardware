/*
  Serial Event example

  When new serial data arrives, this sketch adds it to a String.
  When a newline is received, the loop prints the string and clears it.

  A good test for this is to try it with a GPS receiver that sends out
  NMEA 0183 sentences.

  NOTE: The serialEvent() feature is not available on the Leonardo, Micro, or
  other ATmega32U4 based boards.

  created 9 May 2011
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/SerialEvent
*/

#include <Wire.h>
#include <hd44780.h>                      
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <ArduinoJson.h>

#define LED_RED       4
#define LED_GREEN     5
#define LAMP_SWITCH   6
#define DOOR_SWITCH   7
#define LAMP_PIN      8
#define DOOR_PIN      9

hd44780_I2Cexp lcd;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

const char* response_status = "";
bool access = false;
const char* time_attend = "";
const char* name = "";
const char* message = "";
int type = 0;

int lamp_state                = LOW;
int lamp_switch_state         = LOW;
int recent_lamp_switch_state  = LOW;

unsigned long last_debounce_time  = 0;
unsigned long debounce_delay      = 50;

void setup() {
  // initialize serial:
  Serial.begin(9600);
  lcd.begin(16, 2);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  pinMode(LAMP_SWITCH, INPUT_PULLUP);
  pinMode(DOOR_SWITCH, INPUT_PULLUP);
  
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(DOOR_PIN, OUTPUT);
  
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  digitalWrite(LAMP_PIN, lamp_state);
  digitalWrite(DOOR_PIN, LOW);
}

void loop() {
  // print the string when a newline arrives:
  unsigned long currentMillis = millis();
  lcd.setCursor(0, 0);
  lcd.print("   SMART DOOR   ");
  lcd.setCursor(0, 1);
  lcd.print(" LOCKING SYSTEM ");
  if (stringComplete) {
    
    json_decode(inputString);

    if (response_status[0] == 's') {
      if (access) {
        if (type == 1) {
          lcd.clear();
          digitalWrite(LED_GREEN, HIGH);
          digitalWrite(DOOR_PIN, HIGH);
          delay(500);
          digitalWrite(LED_GREEN, LOW);
          showText("Welcome, Lecturer!", name, 3000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("  Time Attend:  ");
          lcd.setCursor(0, 1);
          lcd.print("     ");
          lcd.print(time_attend);
          lcd.print("     ");
          delay(2000);
          digitalWrite(DOOR_PIN, LOW);
          lcd.clear();
          if (lamp_state != HIGH) {
            lamp_state = HIGH;
          }
        } else if (type == 2) {
          lcd.clear();
          digitalWrite(LED_GREEN, HIGH);
          digitalWrite(DOOR_PIN, HIGH);
          delay(500);
          digitalWrite(LED_GREEN, LOW);
          showText("Welcome, Student!", name, 3000);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("  Time Attend:  ");
          lcd.setCursor(0, 1);
          lcd.print("     ");
          lcd.print(time_attend);
          lcd.print("     ");
          delay(2000);
          digitalWrite(DOOR_PIN, LOW);
          lcd.clear();
        } else {
          digitalWrite(LED_GREEN, HIGH);
          digitalWrite(DOOR_PIN, HIGH);
          delay(500);
          digitalWrite(LED_GREEN, LOW);
          lcd.clear();
          showText("Welcome ", message, 3000);
          delay(2000);
          digitalWrite(DOOR_PIN, LOW);
          lcd.clear();
        }
      }
    } else if (response_status[0] == 'f') {
      digitalWrite(LED_RED, HIGH);
      lcd.clear();
      showText("Failed: ", message, 2000);
      digitalWrite(LED_RED, LOW);
      lcd.clear();
    }

    Serial.print("Time: ");
    Serial.print(currentMillis);
    Serial.print("\t");
    Serial.print("Status: ");
    Serial.println(response_status);
    
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  
  if (digitalRead(DOOR_SWITCH) == HIGH) {
    doorOpen();
  }

  int lamp_reading = digitalRead(LAMP_SWITCH);
  if (lamp_reading != recent_lamp_switch_state) {
    last_debounce_time = millis();
  }

  if ((millis() - last_debounce_time) > debounce_delay) {
    if (lamp_reading != lamp_switch_state) {
      lamp_switch_state = lamp_reading;

      if (lamp_switch_state == HIGH) {
        lamp_state = !lamp_state;
      }
    }
  }

  digitalWrite(LAMP_PIN, lamp_state);
  recent_lamp_switch_state = lamp_reading;
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

void json_decode(String json) {
  const size_t capacity = JSON_OBJECT_SIZE(6) + 110;
  DynamicJsonDocument doc(capacity);
  
  deserializeJson(doc, json);

  response_status = doc["status"];
  access = doc["access"];
  time_attend = doc["time_attend"];
  name = doc["name"];
  type = doc["type"];
  message = doc["message"];
}

void doorOpen() {
  digitalWrite(DOOR_PIN, HIGH);
  delay(3000);
  digitalWrite(DOOR_PIN, LOW);
  delay(3000);
}

void showText(String firsttext, String secondtext, long totalDelay) {
  const long delayShow = 1500;
  long delayRemains = totalDelay;
  long delayPerChar;

  lcd.setCursor(0, 0);
  lcd.print(firsttext);
  if (secondtext.length() > 16) {
    lcd.setCursor(0, 1);
    for (byte positionCounter = 0; positionCounter <= 16; positionCounter++) {
      lcd.write(secondtext[positionCounter]);
    }
    delay(delayShow);
    delayRemains -= delayShow;
    for (byte positionCounter = 17; positionCounter <= secondtext.length(); positionCounter++) {
      lcd.write(secondtext[positionCounter]);
      lcd.scrollDisplayLeft();
      lcd.setCursor(positionCounter - 16, 0);
      lcd.print(firsttext);
      lcd.setCursor(positionCounter + 1, 1);
      delayPerChar = (long) ((delayRemains) / (secondtext.length() - 16));
      delay(delayPerChar);
    }
    lcd.clear();
  } else {
    lcd.setCursor(0, 1);
    lcd.print(secondtext);
    delay(totalDelay);
    lcd.clear();
  }
}
