#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27,16,2);
RTC_DS3231 rtc;

int buttonTogglePin = 2;
int buttonAlarmPin  = 4;
int buzzerPin       = 3;

bool showGoodWord = false;
int alarmCountdown = 0;
bool alarmActive = false;

const char* goodWords[] = {
  "Keep Smiling!",
  "Have a Nice Day!",
  "Stay Positive!",
  "You are Awesome!"
};
int goodWordsCount = 4;

void setup() {
  Wire.begin();
  lcd.init();
  lcd.backlight();
  pinMode(buttonTogglePin, INPUT_PULLUP);
  pinMode(buttonAlarmPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  if (!rtc.begin()) {
    lcd.setCursor(0,0);
    lcd.print("RTC not found!");
    while(1);
  }

  lcd.setCursor(0,0);
  lcd.print("Arduino Clock");
  delay(2000);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  if(digitalRead(buttonTogglePin) == HIGH){
    showGoodWord = !showGoodWord;
    delay(200);
  }

  if(digitalRead(buttonAlarmPin) == HIGH){
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
