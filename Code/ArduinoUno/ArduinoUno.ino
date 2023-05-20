#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#include <Stepper.h>
#include <OneWire.h>
#include <String.h>

SoftwareSerial Arduino_SoftSerial(10, 11);  //RX, TX

#define PIN 13
#define NUM_LEDS 12

Adafruit_NeoPixel pixels(NUM_LEDS, PIN, NEO_GRB);

// Setup Timer
unsigned long previousMillis = 0;
const long timerDelay = 60000;  //(1phut)

char c;
String dataIn;
char dataInArr[100];

const int stepsPerRevolution = 2048;
const int in1Pin = 2;
const int in2Pin = 3;
const int in3Pin = 4;
const int in4Pin = 5;

Stepper myStepper(stepsPerRevolution, in1Pin, in2Pin, in3Pin, in4Pin);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Arduino_SoftSerial.begin(9600);
  myStepper.setSpeed(5);

  pinMode(LED_BUILTIN, OUTPUT);
  pixels.begin();
}

int foodDelay = 100000000;
int foodDelayTmp = 100000000;
void loop() {
  unsigned long currentMillis = millis();
  //   put your main code here, to run repeatedly:
  while (Arduino_SoftSerial.available() > 0) {
    c = Arduino_SoftSerial.read();
    if (c == '\n') {
      break;
    } else {
      dataIn += c;
    }
  }
  if (c == '\n') {
    Serial.println(dataIn);
    if (dataIn.equalsIgnoreCase("2")) {
      myStepper.step(stepsPerRevolution / 4);
    }
    if (dataIn.equalsIgnoreCase("1-1")) {
      for (int i = 0; i < NUM_LEDS; i++) {
        pixels.setPixelColor(i, 200, 200, 200);  // set all LEDs to red
        pixels.show();                           // update the LED ring
      }
    }
    if (dataIn.equalsIgnoreCase("1-0")) {
      for (int i = 0; i < NUM_LEDS; i++) {
        pixels.setPixelColor(i, 0, 0, 0);  // set all LEDs to red
        pixels.show();                     // update the LED ring
      }
    }
    //Reset the variable
    c = 0;
    dataIn = "";
  }
}