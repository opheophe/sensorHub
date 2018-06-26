#include <Wire.h>
#include <Rtc_Pcf8563.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <Servo.h>
#include <Math.h>


//init the real time clock
Rtc_Pcf8563 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4);

// INIT IR-reciever
int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;

// Servo
int servo1_pin = 12;
int servo2_pin = 11;
Servo servo1;
Servo servo2;

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)

const int backlight_toggle = 3;
int backlight_state = 0;
int screen_counter = 0;
int set_clock_stage = 0;
int set_clock_hour = 0;
int set_clock_min = 0;
int set_clock_sec = 0;
long millismod = 0;

int hour;
int minute;
int second;

boolean trigger_close = false;
boolean trigger_open = false;


void closeBlinds() {
  Serial.println("Closing");
  lcdPrint(0, 1, "Closing blinds  ");
  moveto(1, 0);
  moveto(2, 180);
  lcdPrint(0, 1, "                ");
  Serial.println("Closing : Done");
}

void openBlinds() {
  Serial.println("Opening");
  lcdPrint(0, 1, "Opening blinds  ");
  moveto(1, 180);
  moveto(2, 0);
  lcdPrint(0, 1, "                ");
  Serial.println("Opening : Done");
}

void halfBlinds() {
  Serial.println("Half-open blinds");
  lcdPrint(0, 1, "Half-open blinds");
  moveto(1, 90);
  moveto(2, 90);
  lcdPrint(0, 1, "                ");
  Serial.println("Opening : Done");
}


void setup()
{
  setMillisMod();

  // INIT LCD
  lcd.init();
  lcd.backlight();
  lcdPrint(0, 0, "                ");
  lcdPrint(0, 1, "                ");

  // INIT IR recieve
  irrecv.enableIRIn();

  // Allows toggling of backligt
  pinMode(backlight_toggle, INPUT);
  Serial.begin(9600);

}

void setMillisMod() {
  millismod = rtc.getSecond() * 1L + rtc.getMinute() * 1 * 60L + rtc.getHour() * 1 * 60 * 60L;
}

void printMillisTime(long val) {
  int hours = numberOfHours(val);
  int minutes = numberOfMinutes(val);
  int seconds = numberOfSeconds(val);
  char buffer[50];
  sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
  lcdPrint(0, 1, String(buffer));
}

void moveto(int servonum, int angle) {
  if (servonum == 1) {
    servo1.attach(servo1_pin);
    servo1.write(angle);
    delay(1000);
    servo1.detach();
  } else {
    servo2.attach(servo2_pin);
    servo2.write(angle);
    delay(1000);
    servo2.detach();
  }
}




String decode_value(unsigned long input) {
  switch (input)  {
    case 0xFF6897: //1
      closeBlinds();
      break;
    case 0xFF9867: //2
      halfBlinds();
      break;
    case 0xFFB04F: // 3
      openBlinds();
      break;
    case 0xFF30CF: // 4
      break;
    case 0xFF18E7: // 5
      break;
    case 0xFF7A85: // 6
      break;
    case 0xFF10EF: // 7
      break;
    case 0xFF38C7: // 8
      break;
    case 0xFF5AA5: // 9
      break;
    case 0xFF4AB5: // 0
      break;
    case 0xFF42BD: // *
      break;
    case 0xFF52AD: // #
      Serial.println("Backlight : With remote : Start");
      toggleBacklight();
      Serial.println("Backlight : With remote : End");
      break;
    case 0xFF02FD: // OK
      break;
    case 0xFF22DD:
      Serial.println("LEFT ");
      break;
    case 0xFF629D: // Up
      Serial.println("UP   ");
      break;
    case 0xFFC23D:
      Serial.println("RIGHT");
      break;
    case 0xFFA857: //*
      break;
  }
}

void lcdPrint(int col, int row, String text) {
  lcd.setCursor(col, row);
  lcd.print(text);
  delay(50);
}

void lcdPrint(int col, int row, int text) {
  lcd.setCursor(col, row);
  lcd.print(text);
  delay(50);
}

void lcdPrint(int col, int row, long text) {
  delay(200);
  lcd.setCursor(col, row);
  lcd.print(text);
  delay(200);
}

void lcdAppend(String text) {
  delay(200);
  lcd.print(text);
  delay(200);
}

void lcdAppend(int text) {
  delay(200);
  lcd.print(text);
  delay(200);
}

void lcdAppend(long text) {
  delay(200);
  lcd.print(text);
  delay(200);
}

void lcdClear() {
  lcdPrint(0, 0, "                ");
  lcdPrint(0, 1, "                ");
}

void toggleBacklight() {
  Serial.println("Backlight : Start");
  delay(1000);
  Serial.println("Backlight : Delay done");
  if (backlight_state == 0) {
    Serial.println("Backlight : Off");
    backlight_state = 1;
    lcd.noBacklight();
    Serial.println("Backlight : Off : End");
  } else {
    Serial.println("Backlight : On");
    backlight_state = 0;
    lcd.backlight();
    Serial.println("Backlight : On : End");
  }
  delay(1000);
  Serial.println("Backlight : End");
}

void loop()
{
  if (digitalRead(backlight_toggle)) {
    Serial.println("Backlight : With button : Start");
    toggleBacklight();
    Serial.println("Backlight : With button : End");
  }

  lcdPrint(0, 0, rtc.formatTime());

  if (irrecv.decode(&results)) {
    Serial.println(results.value);
    decode_value(results.value);
    irrecv.resume();
  }

  hour = rtc.getHour();
  minute = rtc.getMinute();
  second = rtc.getSecond();

  // Trigger close
  if (((hour == 21) && (minute == 30)) && trigger_close == false) {
    Serial.println("Close timer : Triggering");
    closeBlinds();
    trigger_close = true;
  } else if (((hour == 21) && (minute == 31)) && trigger_close == true) {
    trigger_close = false;
    Serial.println("Close timer : Resetting");
  }

  // Trigger open
  if (((hour == 8) && (minute == 0)) && trigger_open == false) {
    Serial.println("Open timer : Triggering");
    openBlinds();
    trigger_open = true;
  } else if (((hour == 8) && (minute == 1)) && trigger_open == true) {
    trigger_open = false;
    Serial.println("Open timer : Resetting");
  }


}
