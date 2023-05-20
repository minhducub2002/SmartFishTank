#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_NeoPixel.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>
#include <time.h>
#include <Wire.h>

SoftwareSerial NodeMcu_SoftSerial(D3, D4);  //RX,TX

// Khai bao nhiet do DS18B20
const int oneWireBus = 4;  //D2

// Khai bao relay
const int Bom = D0;  //Tx
const int Sui = D1;  //Rx
const int Suoi = D5;
const int Loc = D6;

//Khai bao SR04
const int trigPin = 13;  //D7
const int echoPin = 15;  //D8

// Trang thai cua relay
int suiValue = 0;
int locValue = 0;
int suoiValue = 0;
int bomValue = 0;

// Khai bao khoang cach (cm/uS)
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701
long duration;
float distanceCm;
float distanceInch;

// Setup DS18B20
OneWire oneWire(oneWireBus);
DallasTemperature DS18B20(&oneWire);
float temperatureC;

BlynkTimer timer;

// Setup Blynk
char auth[] = "wzv_dRTtL8701gqHCHi4fNrzgvvYSdMV";
//char auth[] = "1IkF1ogb2fn24eG5o0qGWFYbt1PTbVus";
//char auth[] = "SE_Q9jrXkRszff8tsz6SrgF7AfvIfFn9";

// // Setup Wifi
// char ssid[] = "P311";
// char pass[] = "29102002";
//char ssid[] = "IoT1603";
//char pass[] = "123123123";
// char ssid[] = "HayTuLapDi";
// char pass[] = "haytulapdi";
char ssid[] = "Truong_thanh_len";
char pass[] = "muoinghin";
// char ssid[] = "P104";
// char pass[] = "sh104105";

// Setup Timer
unsigned long previousMillis = 0;
const long timerDelay = 500;

// Setup Canh bao
int height = 0;
int waterMAX = 0;
int waterMIN = 0;
float waterLevel = 0;
int waterLevelInPercent;
int check = 1;
int tempMAX = 0;
int timeTemp = 0;

//Lay thoi gian thuc
int timezone = 7 * 3600;
int dst = 0;
int ngay, thang, nam, gio, phut, giay;

void setup() {
  Serial.begin(9600);
  NodeMcu_SoftSerial.begin(9600);

  pinMode(Bom, OUTPUT);
  pinMode(Sui, OUTPUT);
  pinMode(Loc, OUTPUT);
  pinMode(Suoi, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  DS18B20.begin();

  Blynk.begin(auth, ssid, pass, IPAddress(13, 215, 175, 33), 8080);  //server anh Đức Anh
  //  Blynk.begin(auth, ssid, pass, IPAddress(3, 128, 107, 74), 12258);
  //Blynk.begin(auth, ssid, pass, IPAddress(192, 168, 0, 100), 8080); //server cá nhân

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for server");
  while (!time(nullptr)) {
    Serial.print("#");
    delay(1000);
  }
  Serial.println("Time respone is ok!: ");
}

BLYNK_CONNECTED() {
  Blynk.syncAll();
}
BLYNK_WRITE(V1) {
  bomValue = param.asInt();  // Trạng thái của nút điều khiển (1 là bật, 0 là tắt)
  digitalWrite(Bom, bomValue);
}
BLYNK_WRITE(V2) {
  suiValue = param.asInt();  // Trạng thái của nút điều khiển (1 là bật, 0 là tắt)
  digitalWrite(Sui, suiValue);
}
BLYNK_WRITE(V3) {
  locValue = param.asInt();  // Trạng thái của nút điều khiển (1 là bật, 0 là tắt)
  digitalWrite(Loc, locValue);
}
BLYNK_WRITE(V4) {
  suoiValue = param.asInt();  // Trạng thái của nút điều khiển (1 là bật, 0 là tắt)
  digitalWrite(Suoi, suoiValue);
}
int foodTemp = 0;
BLYNK_WRITE(V11) {
  int food = param.asInt();  // Trạng thái của nút điều khiển (1 là bật, 0 là tắt)
  if (food != foodTemp) {
    if (food == 1) {
      NodeMcu_SoftSerial.print("2\n");
    }
    foodTemp = food;
  }
}
BLYNK_WRITE(V5) {
  int ledValue = param.asInt();  // Trạng thái của nút điều khiển (1 là bật, 0 là tắt)
  if (ledValue == 1) {
    NodeMcu_SoftSerial.print("1-1\n");
  }
  if (ledValue == 0) {
    NodeMcu_SoftSerial.print("1-0\n");
  }
}
BLYNK_WRITE(V14) {
  height = param.asInt();
}
BLYNK_WRITE(V15) {
  waterMIN = param.asInt();
}
BLYNK_WRITE(V16) {
  waterMAX = param.asInt();
}
BLYNK_WRITE(V13) {
  tempMAX = param.asInt();
}
String foodTime;
BLYNK_WRITE(V17) {
  TimeInputParam t(param);
  if (t.hasStartTime()) {
    foodTime = t.getStartHour();
    foodTime = foodTime + ":";
    foodTime = foodTime + t.getStartMinute();
  }
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= timerDelay) {
    previousMillis = currentMillis;
    //Do khoang cach
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculate the distance
    distanceCm = duration * SOUND_VELOCITY / 2;

    waterLevel = height - distanceCm;
    waterLevelInPercent = (height - distanceCm) / waterMAX * 100;

    //Do nhiet do
    DS18B20.requestTemperatures();
    temperatureC = DS18B20.getTempCByIndex(0);

    //Thu thap data
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);
    // if (check == 0) {
    //   Serial.println("STT ThoiGian NhietDo MucNuoc");
    //   check++;
    // }
    ngay = p_tm->tm_mday;
    thang = p_tm->tm_mon + 1;
    nam = p_tm->tm_year + 1900;
    gio = p_tm->tm_hour;
    phut = p_tm->tm_min;
    giay = p_tm->tm_sec;
    // String data = "";
    //  data = data + String(check);
    //  data = data + " ";
    // data = data + String(ngay);
    // data = data + "/";
    // data = data + String(thang);
    // data = data + "/";
    // data = data + String(nam);
    // data = data + "-";
    // data = data + String(gio);
    // data = data + ":";
    // data = data + String(phut);
    // data = data + ":";
    // data = data + String(giay);
    //    data = data + " ";
    //    data = data + String(temperatureC);
    //    data = data + " ";
    //    data = data + String(waterLevel);
    //    data = data + " ";
    //    check++;
    //  Serial.println(data);
    // Serial.print("chieu cao be: ");
    // Serial.println(height);
    // Serial.print("setup muc nuoc Max: ");
    // Serial.println(waterMAX);
    // Serial.print("setup nhiet do Max: ");
    // Serial.println(tempMAX);
    String time = "";
    time = time + String(gio);
    time = time + ":";
    time = time + String(phut);
    // time = time + ":";
    // time = time + String(giay);
    Serial.print("Time: ");
    Serial.println(time);
    Serial.print("FoodTime: ");
    Serial.println(foodTime);
    if (foodTime.equalsIgnoreCase(time) && check == 1) {
      NodeMcu_SoftSerial.print("2\n");
      Serial.println("OK!!!!!");
      timeTemp = phut + 1;
      check = 0;
    }
    Serial.print("temp: ");
    Serial.println(timeTemp);
    Serial.print("check: ");
    Serial.println(check);
    if (phut == timeTemp) {
      Serial.println("Reset!");
      timeTemp = 0;
      check = 1;
    }

    //Ban len blynk
    if (waterLevelInPercent <= 0) {
      Blynk.virtualWrite(V8, 0);
    } else if (waterLevelInPercent >= 100) {
      Blynk.virtualWrite(V8, 100);
    } else {
      Blynk.virtualWrite(V8, waterLevelInPercent);
    }
    if (waterLevel >= 0) {
      Blynk.virtualWrite(V9, waterLevel);
    } else {
      Blynk.virtualWrite(V9, 0);
    }
    Blynk.virtualWrite(V10, temperatureC);

    if (waterLevel <= (waterMIN + 1)) {
      Blynk.virtualWrite(V6, "Water is low!");
      Blynk.virtualWrite(V1, 0);
      bomValue = 0;
      digitalWrite(Bom, bomValue);
    } else if (waterLevel > waterMAX) {
      Blynk.virtualWrite(V6, "Water is high!");
    } else {
      Blynk.virtualWrite(V6, "Water ok.");
    }

    if (temperatureC >= tempMAX + 1) {
      Blynk.virtualWrite(V7, "Temp is high!");
      Blynk.virtualWrite(V4, 0);
      suoiValue = 0;
      digitalWrite(Suoi, suoiValue);

    } else if (temperatureC <= (tempMAX - 1)) {
      Blynk.virtualWrite(V7, "Temp is low!");
      suoiValue = 1;
      digitalWrite(Suoi, suoiValue);
      Blynk.virtualWrite(V4, 1);
    } else {
      Blynk.virtualWrite(V7, "Temp ok.");
    }
    if (temperatureC >= (tempMAX + 2)) {
      Blynk.notify("Temperature is too high!!!");
      Blynk.virtualWrite(V4, 0);
    }
  }
}
