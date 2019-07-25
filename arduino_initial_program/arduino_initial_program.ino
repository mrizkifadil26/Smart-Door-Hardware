#include <Servo.h>

const boolean DEBUG = false;

const int SERVO_PIN = 10;
const int LAMP_PIN = 4;
const int LAMP_SWITCH = 5;
const int DOOR_SWITCH = 6;
const int LED_RED = 9;
const int LED_GREEN = 8;
const int BUZZER_PIN = 7;

bool isDoorOpen = false;
bool isLampOn = false;

int doorState = LOW;
int lampState = LOW;

int lampStateCounter = 0;

int doorFlag = 0;
int lampFlag = 0;

int servoPosition = 0;

Servo doorLock;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    return;
  }
  
  pinMode(LAMP_SWITCH, INPUT_PULLUP);
  pinMode(DOOR_SWITCH, INPUT_PULLUP);

  pinMode(LAMP_PIN, OUTPUT);
  
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  doorLock.attach(SERVO_PIN);
}

void loop() {
  // put your main code here, to run repeatedly:
  doorState = digitalRead(DOOR_SWITCH);
  lampState = digitalRead(LAMP_SWITCH);

  //======  LAMP SERIAL COMMAND =======//
  if (Serial.available() > 0) {
    char state = Serial.read();
    if (state == 'H' || state == 'h') {
      lampOn(DEBUG);
      delay(5000);
    }
    if (state == 'L' || state == 'l') {
      lampOff(DEBUG);
      delay(5000);
    }
  }
  delay(50);
  //===================================//
  
  //======  LAMP MANUAL SWITCH =======//
  if (lampState != lampFlag) {
    if (lampState == HIGH) {
      lampStateCounter++;
    }
    delay(50);
  }

  lampFlag = lampState;

  if (lampStateCounter % 2 == 0) {
    lampOff();
  } else {
    lampOn();
  }
  //==================================//
  
}

void lampOn() {
  digitalWrite(LAMP_PIN, HIGH);
  isLampOn = true;
}

void lampOff() {
  digitalWrite(LAMP_PIN, LOW);
  isLampOn = false;
}

void doorOpen() {
  for (servoPosition = 0; servoPosition <= 90; servoPosition += 1) {
    doorLock.write(servoPosition);
    delay(15);
  }
  delay(5000);
  for (servoPosition = 90; servoPosition >= 0; servoPosition -= 1) {
    doorLock.write(servoPosition);
    delay(15);
  }
  
  isDoorOpen = true;
//  showSerial("Door is Open");
}

void doorClose() {
  isDoorOpen = false;
//  showSerial("Door is Closed");
}

void showText(String text) {
  
}

void showSerial(String text) {
  Serial.println(text);
}
