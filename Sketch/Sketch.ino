#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <TwitterApi.h>
#include <SimpleTimer.h>

#define BEARER_TOKEN ""

const char* ssid = "";
const char* password = "";

WiFiClientSecure client;
TwitterApi api(client);
SimpleTimer timer;

unsigned long api_mtbs = 60000; //mean time between api requests
unsigned long api_lasttime = 0;   //last time api request has been done

String tweet = "";
int lineNum;
String lines[4];
int scrollDelay = 1500;
bool printNext;

LiquidCrystal lcd(13, 12, 14, 5, 4, 2); //V2

void setup()
{
  Serial.begin(9600);

  lcd.begin(20, 4);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("WiFi Connecting");
  lcd.setCursor(0, 1);
  lcd.print("SSID: ");
  lcd.print(ssid);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  lcd.setCursor(0, 2);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }

  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  lcd.setCursor(0, 2);
  lcd.print("WiFi Connected      ");
  lcd.setCursor(0, 3);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());

  api.setBearerToken(BEARER_TOKEN);

  printNext = true;
  tweet = "";

  timer.setInterval(100, getTweet);
  timer.setInterval(100, printTweet);
}

void getTweet() {
  if ((millis() > api_lasttime + api_mtbs))  {
    Serial.println("Getting Tweet");
    String command = "/1.1/statuses/user_timeline.json?screen_name=realDonaldTrump&count=1";
    String responseString = api.sendGetToTwitter(command);
    const size_t capacity = 13718;
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonArray& response = jsonBuffer.parseArray(responseString);
    if (response.success()) {
      String t = response[0]["text"];
      tweet = t;
    }
    else {
      Serial.println("Failed to parse Json");
    }
    api_lasttime = millis();
  }
}

void printTweet() {
  if (tweet.length() == 0) return;
  if (printNext) {
    printNext = false;
    lcd.clear();

    char buf[280];
    tweet.toCharArray(buf, 280);
    char *p = buf;
    char *str;
    String line = "";
    while ((str = strtok_r(p, " ", &p)) != NULL) {
      String nextWord = String(str);
      if (nextWord.substring(0, 4) == "http") {
        nextWord = "";
      }
      if (line.length() + nextWord.length() < 20) {
        line += nextWord + " ";
      }
      else {
        lines[0] = lines[1];
        lines[1] = lines[2];
        lines[2] = lines[3];
        lines[3] = line;

        lcd.clear();
        for (int i = 0; i < 4; i++) {
          lcd.setCursor(0, i);
          lcd.print(lines[i]);
          Serial.println(lines[i]);
        }
        if (nextWord.length() > 19) {
          nextWord = nextWord.substring(0, 19);
        }
        line = nextWord + " ";
        delay(scrollDelay);
      }
    }
    for (int i = 0; i < 4; i++) {
      lines[0] = lines[1];
      lines[1] = lines[2];
      lines[2] = lines[3];
      lines[3] = line;

      lcd.clear();
      for (int i = 0; i < 4; i++) {
        lcd.setCursor(0, i);
        lcd.print(lines[i]);
        Serial.println(lines[i]);
      }
      line = "";
      delay(scrollDelay);
    }
    lcd.clear();
    printNext = true;
  }
}

void loop() {
  timer.run();
}
