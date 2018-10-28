#include <Wire.h>
#include <Rtc_Pcf8563.h>
#include <IRremote.h>
#include <Servo.h>
#include <Math.h>


//init the real time clock
Rtc_Pcf8563 rtc;
int led_pin=4;

// INIT IR-reciever
int RECV_PIN = 2;
IRrecv irrecv(RECV_PIN);
decode_results results;

// Servo
int servo1_pin = 12;
int servo2_pin = 13;
Servo servo1;
Servo servo2;

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)

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
  moveto(1, 0);
  moveto(2, 180);
  Serial.println("Closing : Done");
}

void openBlinds() {
  Serial.println("Opening");
  moveto(1, 180);
  moveto(2, 0);
  Serial.println("Opening : Done");
}

void halfBlinds() {
  Serial.println("Half-open blinds");
  moveto(1, 90);
  moveto(2, 90);
  Serial.println("Opening : Done");
}


void setup()
{
  setMillisMod();

  // INIT IR recieve
  irrecv.enableIRIn();
  pinMode(led_pin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting");
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
  Serial.println(String(buffer));
}

void moveto(int servonum, int angle) {
  if (servonum == 1) {
    servo1.attach(servo1_pin);
    servo1.write(angle);
    delay(750);
    servo1.detach();
    delay(250);
  } else {
    servo2.attach(servo2_pin);
    servo2.write(angle);
    delay(750);
    servo2.detach();
    delay(250);
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



void loop()
{
  if (irrecv.decode(&results)) {
    digitalWrite(led_pin,HIGH);
    Serial.println(results.value);
    decode_value(results.value);
    irrecv.resume();
     delay(25);
     digitalWrite(led_pin,LOW);
  }

  // This is needed for the clock to update
 // rtc.formatTime();
  Serial.println(rtc.formatTime());

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
  if (((hour == 8) && (minute == 10)) && trigger_open == false) {
    Serial.println("Open timer : Triggering");
    openBlinds();
    trigger_open = true;
  } else if (((hour == 8) && (minute == 11)) && trigger_open == true) {
    trigger_open = false;
    Serial.println("Open timer : Resetting");
  }


}

