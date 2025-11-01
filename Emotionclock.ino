#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <WiFiS3.h>
#include <WiFiServer.h>

LiquidCrystal_I2C lcd(0x27,16,2);
RTC_DS3231 rtc;

int buttonTogglePin = 2;
int buttonAlarmPin  = 4;
int buzzerPin       = 3;

bool showGoodWord = false;
int alarmCountdown = 0;
bool alarmActive = false;

const char* ssid = "NailoM";
const char* password = "29102548";

WiFiServer server(80);

String goodWords[30] = {
  "Keep Smiling!",
  "Have a Nice Day!",
  "Stay Positive!",
  "You are Awesome!",
  "Believe in Yourself!",
  "You've Got This!",
  "Make it Count!",
  "Today is Your Day!",
  "Shine On!",
  "Dream Big!",
  "Never Give Up!",
  "Be Happy!",
  "Do Your Best!",
  "Keep Going!",
  "You Can Do It!",
  "Stay Strong!",
  "Trust the Process!",
  "Be Kind!",
  "Live Your Passion!",
  "Focus on the Good!"
};
int goodWordsCount = 20;

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pinMode(buttonTogglePin, INPUT_PULLUP);
  pinMode(buttonAlarmPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(115200);

  if (!rtc.begin()) {
    lcd.setCursor(0,0);
    lcd.print("RTC not found!");
    while(1);
  }

  //rtc.adjust(DateTime(2025,11,1,15,57,30));

  lcd.setCursor(0,0);
  lcd.print("WiFi Connecting");

  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  if(WiFi.status() == WL_CONNECTED){
    lcd.print("WiFi Connected!");
    Serial.println("Connected to WiFi");
    Serial.print("Server IP: ");
    Serial.println(WiFi.localIP());
  } else {
    lcd.print("WiFi Failed!");
    Serial.println("WiFi Failed!");
  }

  delay(1000);
  lcd.clear();
  lcd.print("Emo Clock Ready");

  server.begin();
}

void loop() {
  DateTime now = rtc.now();

  if(digitalRead(buttonTogglePin) == LOW){
    showGoodWord = !showGoodWord;
    delay(200);
  }

  if(digitalRead(buttonAlarmPin) == LOW){
    alarmCountdown += 60;
    delay(200);
  }

  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,0);
  if(showGoodWord){
    int idx = random(goodWordsCount);
    lcd.print(goodWords[idx]);
  } else {
    lcd.setCursor(3,0);
    if(now.day() < 10) lcd.print("0");
    lcd.print(now.day());
    lcd.print("/");
    if(now.month() < 10) lcd.print("0");
    lcd.print(now.month());
    lcd.print("/");
    lcd.print(now.year());
  }

  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  if(alarmCountdown > 0){
    int minutes = alarmCountdown / 60;
    int seconds = alarmCountdown % 60;
    lcd.print("Alarm in: ");
    print2digits(minutes);
    lcd.print(":");
    print2digits(seconds);

    alarmCountdown--;

    if(alarmCountdown <= 0 && !alarmActive){
      alarmActive = true;
      triggerAlarm();
    }
  } else {
    lcd.setCursor(1,1);
    lcd.print("Time: ");
    print2digits(now.hour());
    lcd.print(":");
    print2digits(now.minute());
    lcd.print(":");
    print2digits(now.second());
    alarmActive = false;
  }

  handleWebServer();
  delay(1000);
}

void triggerAlarm(){
  for(int i=0;i<10;i++){
    tone(buzzerPin, 1000);
    delay(200);
    noTone(buzzerPin);
    delay(200);
  }
}

void print2digits(int number){
  if(number < 10) lcd.print("0");
  lcd.print(number);
}

// 🌐 Web Server (no time display)
void handleWebServer() {
  WiFiClient client = server.available();
  if (!client) return;

  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/add?word=") != -1) {
    int start = request.indexOf("=") + 1;
    int end = request.indexOf(" ", start);
    String newWord = request.substring(start, end);
    newWord.replace("+", " ");
    if (goodWordsCount < 30) goodWords[goodWordsCount++] = newWord;
  }

  if (request.indexOf("/clear") != -1) {
    goodWordsCount = 0;
  }

  if (request.indexOf("/delete?index=") != -1) {
    int start = request.indexOf("=") + 1;
    int idx = request.substring(start).toInt();
    if(idx >= 0 && idx < goodWordsCount){
      for(int i=idx;i<goodWordsCount-1;i++){
        goodWords[i] = goodWords[i+1];
      }
      goodWordsCount--;
    }
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Emo Clock</title>");
  client.println("<style>");
  client.println("body{font-family:Arial; background: linear-gradient(120deg,#84fab0,#8fd3f4); color:#333; text-align:center;}");
  client.println("h1{color:#fff; text-shadow:2px 2px #555; margin-top:20px;}");
  client.println("form{margin:20px;}");
  client.println("input[type=text]{padding:8px; width:200px; border-radius:5px; border:none;}");
  client.println("input[type=submit]{padding:8px 15px; background:#ff7f50; color:white; border:none; border-radius:5px; cursor:pointer; font-weight:bold;}");
  client.println("input[type=submit]:hover{background:#ff4500;}");
  client.println("ul{list-style:none; padding:0;}");
  client.println("li{background:#ffffff66; margin:10px auto; padding:10px; width:300px; border-radius:10px; box-shadow:0 4px 8px rgba(0,0,0,0.2); font-weight:bold; color:#333; display:flex; justify-content:space-between; align-items:center;}");
  client.println("a.delete{background:#ff3b2e; padding:5px 10px; border-radius:5px; color:#fff; text-decoration:none;}");
  client.println("a.delete:hover{background:#ff7f50;}");
  client.println("</style>");
  client.println("</head><body>");
  client.println("<h1>😊 Emotion Clock 😊</h1>");
  client.println("<form action='/add' method='get'>");
  client.println("<input name='word' placeholder='New positive word'>");
  client.println("<input type='submit' value='Add'>");
  client.println("</form>");
  client.println("<p><a href='/clear'>Clear All Words</a></p>");
  client.println("<ul>");
  for(int i=0;i<goodWordsCount;i++){
    client.println("<li>" + goodWords[i] + " <a class='delete' href='/delete?index=" + i + "'>Delete</a></li>");
  }
  client.println("</ul></body></html>");
  client.stop();
}
