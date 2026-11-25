#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <ESP32Servo.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include <sha1.h>
#include <TOTP.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
String GOOGLE_SCRIPT_ID = "YOUR_GOOGLE_SCRIPT_ID";

Servo myServo;
const int servoPin = 13;
const int buttonPin = 15;

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {19, 18, 17, 16};
byte colPins[COLS] = {32, 33, 25, 26};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

uint8_t hmacKey1[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
TOTP totp1 = TOTP(hmacKey1, 10);

uint8_t hmacKey2[] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
TOTP totp2 = TOTP(hmacKey2, 10);

String inputCode = "";
int lastMinute = -1;
bool isStandby = true;

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Ket noi Wi-Fi...");

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myServo.setPeriodHertz(50);
  myServo.attach(servoPin, 500, 2400);
  myServo.write(0);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("Da ket noi Wi-Fi");
  delay(1000);

  timeClient.begin();

  lcd.clear();
  lcd.print("Dong bo gio...");
  while (timeClient.getEpochTime() < 1000000000) {
    timeClient.forceUpdate();
    delay(1000);
  }

  resetState();
}

void loop() {
  timeClient.update();

  if (digitalRead(buttonPin) == LOW) {
    isStandby = false;
    lcd.clear();
    lcd.print("Da mo cua!");
    lcd.setCursor(0, 1);
    lcd.print("Mo tu ben trong");

    myServo.write(90);
    logToGoogleSheets("Nut_Nhan", "Da_mo_cua");

    delay(5000);
    myServo.write(0);
    resetState();
  }

  if (isStandby) {
    int currentMinute = timeClient.getMinutes();
    if (currentMinute != lastMinute) {
      lastMinute = currentMinute;
      int h = (timeClient.getHours() + 7) % 24;

      lcd.setCursor(11, 1);
      if (h < 10) lcd.print("0");
      lcd.print(h);
      lcd.print(":");
      if (currentMinute < 10) lcd.print("0");
      lcd.print(currentMinute);

      lcd.setCursor(inputCode.length(), 1);
    }
  }

  char key = keypad.getKey();

  if (key) {
    if (key == '*') {
      resetState();
    } else if (key >= '0' && key <= '9') {
      if (inputCode.length() < 6) {
        lcd.setCursor(inputCode.length(), 1);
        inputCode += key;
        lcd.print(key);
      }

      if (inputCode.length() == 6) {
        isStandby = false;
        lcd.clear();
        lcd.print("Dang xac thuc...");
        verifyInputCode();
      }
    }
  }
}

void verifyInputCode() {
  long timestamp = timeClient.getEpochTime();

  String totpAdmin = String(totp1.getCode(timestamp));
  String totpGuest = String(totp2.getCode(timestamp));

  String userName = "";
  bool isAuthorized = false;

  if (inputCode == totpAdmin) {
    userName = "Admin";
    isAuthorized = true;
  } else if (inputCode == totpGuest) {
    userName = "Guest";
    isAuthorized = true;
  }

  if (isAuthorized) {
    lcd.clear();
    lcd.print("Da mo cua!");
    lcd.setCursor(0, 1);
    lcd.print("Chao mung ban <3");

    myServo.write(90);
    logToGoogleSheets(userName, "Da_mo_cua");

    delay(5000);
    myServo.write(0);
    resetState();
  } else {
    lcd.clear();
    lcd.print("Sai ma OTP!");
    delay(2000);
    resetState();
  }
}

void logToGoogleSheets(String user, String action) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure *client = new WiFiClientSecure;
    if (client) {
      client->setInsecure();
      HTTPClient http;

      String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?user=" + user + "&action=" + action;

      http.begin(*client, url);
      int httpCode = http.GET();

      if (httpCode > 0) {
        Serial.println("Ghi log len Sheets thanh cong!");
      }
      http.end();
      delete client;
    }
  }
}

void resetState() {
  inputCode = "";
  lcd.clear();
  lcd.print("Moi nhap OTP:");

  lastMinute = -1;
  isStandby = true;
}