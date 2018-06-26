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



// These are used for the internal clock
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)

const int backlight_toggle = 3;
int backlight_state = 0;
int current_screen = 0;
int screen_counter = 0;
int set_clock_stage = 0;
int set_clock_hour = 0;
int set_clock_min = 0;
int set_clock_sec = 0;
long millismod = 0;

void closeBlinds() {
  Serial.println("Closing");
  //lcdPrint(0, 1, "Closing blinds  ");
  moveto(1, 0);
  moveto(2, 180);
  Serial.println("Closing : Done");
}

void openBlinds() {
  Serial.println("Opening");
  //lcdPrint(0, 1, "Opening blinds  ");
  moveto(1, 180);
  moveto(2, 0);
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
      current_screen = 1;
      closeBlinds();
      break;
    case 0xFF9867: //2
      current_screen = 1;
      Serial.println("Turning 90   ");
      lcdPrint(0, 1, "Turning 90   ");
      moveto(1, 90);

      moveto(2, 90);
      break;
    case 0xFFB04F: // 3
      current_screen = 1;
      openBlinds();
      break;
    case 0xFF30CF:
      Serial.println("4    ");
      break;
    case 0xFF18E7:
      Serial.println("5    ");
      break;
    case 0xFF7A85:
      Serial.println("6    ");
      break;
    case 0xFF10EF:
      Serial.println("7    ");
      break;
    case 0xFF38C7:
      Serial.println("8    ");
      break;
    case 0xFF5AA5:
      Serial.println("9    ");
      break;
    case 0xFF4AB5:
      Serial.println("0    ");
      break;
    case 0xFF42BD: // *
      lcdPrint(0, 1, "SET CLOCK   ");
      set_clock_hour = rtc.getHour();
      set_clock_min = rtc.getMinute();
      set_clock_sec = rtc.getSecond();
      set_clock_stage = 0;
      current_screen = 2;
      break;
    case 0xFF52AD: // #
      if (backlight_state == 0) {
        Serial.println("Backlight : Off");
        backlight_state = 1;
        lcd.noBacklight();
      } else {
        Serial.println("Backlight : On");
        backlight_state = 0;
        lcd.backlight();
      }
      delay(500);
      break;
    case 0xFF02FD:
      Serial.println("OK   ");
      lcdPrint(0, 1, "OK   ");
      if (current_screen == 2) {
        set_clock_stage++;
      }
      break;
    case 0xFF22DD:
      Serial.println("LEFT ");
      break;
    case 0xFF629D:
      Serial.println("UP   ");
      if (current_screen == 2) {
        if (set_clock_stage == 0) { // Set hour
          set_clock_hour++;
          if (set_clock_hour > 23) {
            set_clock_hour = 0;
          }
        } else if (set_clock_stage == 1) { // Set min
          set_clock_min++;
          if (set_clock_min > 59) {
            set_clock_min = 0;
          }
        } else if (set_clock_stage == 2) { // Set Sec
          set_clock_sec++;
          if (set_clock_sec > 59) {
            set_clock_sec = 0;
          }
        }
      }

      break;
    case 0xFFC23D:
      Serial.println("RIGHT");
      break;
    case 0xFFA857: //*
      if (current_screen == 2) {
        if (set_clock_stage == 0) { // Set hour
          set_clock_hour--;
          if (set_clock_hour < 0) {
            set_clock_hour = 23;
          }
        } else if (set_clock_stage == 1) { // Set min
          set_clock_min--;
          if (set_clock_min < 0) {
            set_clock_min = 59;
          }
        } else if (set_clock_stage == 2) { // Set Sec
          set_clock_sec--;
          if (set_clock_sec < 0) {
            set_clock_sec = 59;
          }
        }
      }
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
void checkBacklightButton() {
  if (digitalRead(backlight_toggle)) {
    if (backlight_state == 0) {
      Serial.println("Backlight : Off");
      backlight_state = 1;
      lcd.noBacklight();
    } else {
      Serial.println("Backlight : On");
      backlight_state = 0;
      lcd.backlight();
    }
    delay(1000);
  }
}

void loop()
{

  Serial.println("--> Check backlight button");
  checkBacklightButton();

  Serial.println("--> Print current time");
  lcdPrint(0, 0, rtc.formatTime());

  Serial.println("--> Check remote");
  if (irrecv.decode(&results)) {
    decode_value(results.value);
    screen_counter = 0;
    irrecv.resume();
  }


  //if (current_screen == 1) { // button clicked display
  //    Serial.println("--> Current 1");
  //    Serial.print("Screen counter: ");
  //    Serial.println(screen_counter);
  //    screen_counter++;
  //    if (screen_counter == 50) {
  //      current_screen = 0;
  //      screen_counter = 0;
  //    }
  //    lcdPrint(0, 0, rtc.formatTime());
  //    lcdPrint(9, 0, "        ");
  //  } else if (current_screen == 2) { // Set clock display
  //    Serial.println("--> Current 2");
  //    if (screen_counter == 500) {
  //      current_screen = 0;
  //      screen_counter = 0;
  //    }
  //    screen_counter++;
  //    lcd.setCursor(0, 0);
  //    lcd.print("SET: ");
  //    delay(50);
  //    printDigits(set_clock_hour);
  //    lcd.print(":");
  //    delay(50);
  //    printDigits(set_clock_min);
  //    lcd.print(":");
  //    delay(50);
  //    printDigits(set_clock_sec);
  //    lcd.print("                ");
  //    delay(50);
  //    lcd.setCursor(0, 1);
  //    if (set_clock_stage == 0) {
  //      lcd.print("HOUR U/D --> OK");
  //      delay(50);
  //      lcd.setCursor(0, 5);
  //    } else if (set_clock_stage == 1) {
  //      lcd.print("MIN U/D --> OK");
  //      delay(50);
  //      lcd.setCursor(0, 8);
  //    } else if (set_clock_stage == 2) {
  //      lcd.print("SEC U/D --> OK");
  //      delay(50);
  //      lcd.setCursor(0, 11);
  //    } else if (set_clock_stage == 3) {
  //      rtc.setTime(set_clock_hour, set_clock_min, set_clock_sec);
  //      current_screen = 0;
  //      screen_counter = 0;
  //      set_clock_stage = 0;
  //    }
  //  }

  Serial.println("--> End loop");
}
