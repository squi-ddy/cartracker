#include <SoftwareSerial.h>
#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>

#define RX_PIN 4
#define TX_PIN 5
#define SLEEP_PIN 7
#define ON_OFF 6

#define IR_CSN 3
#define IR_CE 2

#define BUZ_PIN 9
#define LED_PIN 8

#define BLK_DUR 32767

SoftwareSerial IOT(RX_PIN, TX_PIN);
RF24 IR(IR_CE, IR_CSN);

const byte IR_ADDR_R[6] = "00001";
const byte IR_ADDR_W[6] = "00002";

bool blinkOn = true;
bool lightOn = true;
bool buzzerOn = true;
int toNextBlink = BLK_DUR;

void turnOn() {
  //pulse off then on
  digitalWrite(ON_OFF, LOW);
  delay(1050); //At least 1s (datasheet)
  digitalWrite(ON_OFF, HIGH);
  //wait for UART port -> from datasheet, minimum 4.5s. Spam "AT" and wait for "OK"
  while (!IOT.available()) {
    Serial.println("Sent AT signal...");
    IOT.write("AT\r\n");
    delay(1000);
  }
  //read all
  while (IOT.available()) {
    IOT.read();
  }
}

String getGPSData() {
  IOT.write("AT+CGNSPWR=1\r\n");
  IOT.write("AT+CGNSINF\r\n");
  String data = "";
  while (IOT.available()) {
    data += IOT.read();
  }
  return data;
}

void sleepOn() {
  digitalWrite(SLEEP_PIN, HIGH);
}

void sleepOff() {
  digitalWrite(SLEEP_PIN, LOW);
}

void checkIR() {
  if (IR.available()) {
    char text[] = "";
    IR.read(&text, sizeof(text));
    if (text == "GET ALM") {
      char text[] = "0";
      IR.write(&text, sizeof(text));
    }
  }
}

void alarmOn(bool buzzer, bool light) {
  if (buzzer) {
    analogWrite(BUZ_PIN, 512);
  }
  if (light) {
    Serial.println("LGT");
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

void setup() {
  Serial.begin(9600);
  IOT.begin(9600);
  IR.begin();
  IR.openReadingPipe(0, IR_ADDR_R);
  IR.openWritingPipe(IR_ADDR_W);
  IR.setPALevel(RF24_PA_MIN);
  IR.startListening();
  pinMode(ON_OFF, OUTPUT);
  //pinMode(SLEEP_PIN, OUTPUT);
  digitalWrite(ON_OFF, HIGH);
  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, HIGH);
  delay(100);
  turnOn();
  Serial.println("Chip OK");
}

void loop() {
  checkIR();

  if (blinkOn) {
    toNextBlink--;
  }

  if (toNextBlink <= 0) {
    lightOn = !lightOn;
    alarmOn(buzzerOn, lightOn);
    toNextBlink = BLK_DUR;
  }
}
