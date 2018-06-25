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
  lcd.setCursor(0, 1);
  lcd.print("Closing blinds  ");
  moveto(1, 0);
  moveto(2, 180);
}

void openBlinds() {
  Serial.println("Opening");
  lcd.setCursor(0, 1);
  lcd.print("Opening blinds  ");
  moveto(1, 180);
  moveto(2, 0);
}

class Alarm {
    boolean triggered = false;
    boolean active = false;
    int hour = 0;
    int minute = 0;
    int second = 0;
    String action = "";

  public:
    void setup() {

    }

    String getAction() {
      return action;
    }

    void start() {
      active = true;
    }

    void stop() {
      active = false;
    }

    void reset() {
      active = true;
      triggered = false;
    }

    void set(String a, int h, int m, int s) {
      action = a;
      hour = h;
      minute = m;
      second = s;
      triggered = false;
      active = true;
    }

    //    void setIn(String, int h, int m, int s){
    //      hour=rtc.getHour();
    //      minute=rtc.getMinute();
    //
    //
    //
    //
    //    }


    String getAlarm() {
      char buffer[50];
      sprintf(buffer, "%02d:%02d:%02d", hour, minute, second);
      return String(buffer);
    }

    long getTimeLeft() {
      long left = (hour * 60L * 60L + minute * 60L + second * 1L) -
                  (rtc.getHour() * 60L * 60L + rtc.getMinute() * 60L + rtc.getSecond() * 1L);


      if ((left < -60) || (triggered && (left < 0 ) )) {
        left = left + 24 * 60 * 60L;
      }

      return left;
    }

    String getTimeLeftFormat() {
      long time_left = getTimeLeft();
      int hours = numberOfHours(time_left);
      int minutes = numberOfMinutes(time_left);
      int seconds = numberOfSeconds(time_left);
      char buffer[50];
      sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
      return (String(buffer));


    }

    boolean checkAlarm() {
      //      if  ((rtc.getHour() == hour) && rtc.getMinute() == minute) {
      long timeLeft = getTimeLeft();

      if ( (timeLeft < 1) && (timeLeft > -60)) {
        if (triggered) {
          // Do nothing, already triggered
        } else {
          triggered = true;
          //   Serial.println("ALAAAAAAARM");
          if (action == "CLOSE") {
            Serial.print(action);
            Serial.print(": ");
            Serial.print(getTimeLeftFormat());
            Serial.print(": ");
            Serial.print(getTimeLeft());
            Serial.print(" ");
            closeBlinds();
          } else if (action == "OPEN") {
            openBlinds();
          }
          return true;
        }
      } else if (timeLeft < -60) {
        triggered = false;
      }
      return false;
    }


};

Alarm alarmClose;
Alarm alarmOpen;

void setup()
{
  // Init clock
  setRTC_clock();
  setMillisMod();

  // INIT LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  // INIT IR recieve
  irrecv.enableIRIn();

  // Allows toggling of backligt
  pinMode(backlight_toggle, INPUT);
  Serial.begin(9600);
  alarmClose.set("CLOSE", 21, 30, 0);
  alarmOpen.set("OPEN", 8, 0, 0);



}

void setMillisMod() {
  millismod = rtc.getSecond() * 1L + rtc.getMinute() * 1 * 60L + rtc.getHour() * 1 * 60 * 60L;
}

void setRTC_clock() {
  //clear out the registers
  // rtc.initClock();
  //hr, min, sec
  //  rtc.setTime(0, 0, 0);
  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  rtc.setDate(16, 5, 6, 0, 18);

  // Print the date
  // Serial.print(rtc.formatDate());
  // Print the time
  // Serial.print(rtc.formatTime());
  // Serial.print("\r\n");

  // Serial.print("\r\n");
}


void printMillisTime(long val) {
  int hours = numberOfHours(val);
  int minutes = numberOfMinutes(val);
  int seconds = numberOfSeconds(val);
  char buffer[50];
  sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);
  lcd.print(String(buffer));
}

void printDigits(byte digits) {
  // utility function for digital clock display: prints leading 0
  if (digits < 10)
    lcd.print('0');
  lcd.print(digits, DEC);
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
  lcd.setCursor(0, 1);
  switch (input)  {
    case 0xFF6897: //1
      current_screen = 1;
      closeBlinds();
      break;
    case 0xFF9867: //2
      current_screen = 1;
      Serial.println("Turning 90   ");
      lcd.print("Turning 90   ");
      moveto(1, 90);

      moveto(2, 90);
      break;
    case 0xFFB04F: // 3
      current_screen = 1;
      openBlinds();
      break;
    case 0xFF30CF:
      Serial.println("4    ");
      lcd.print("4    ");
      break;
    case 0xFF18E7:
      Serial.println("5    ");
      lcd.print("5    ");
      break;
    case 0xFF7A85:
      Serial.println("6    ");
      lcd.print("6    ");
      break;
    case 0xFF10EF:
      Serial.println("7    ");
      lcd.print("7    ");
      break;
    case 0xFF38C7:
      Serial.println("8    ");
      lcd.print("8    ");
      break;
    case 0xFF5AA5:
      Serial.println("9    ");
      lcd.print("9    ");
      break;
    case 0xFF4AB5:
      Serial.println("0    ");
      lcd.print("0    ");
      break;
    case 0xFF42BD: // *
      lcd.print("SET CLOCK   ");
      set_clock_hour = rtc.getHour();
      set_clock_min = rtc.getMinute();
      set_clock_sec = rtc.getSecond();
      set_clock_stage = 0;
      current_screen = 2;
      break;
    case 0xFF52AD: // #
      Serial.println("Backlight");
      lcd.print("Backlight");
      if (backlight_state == 0) {
        backlight_state = 1;
        lcd.noBacklight();
      } else {
        backlight_state = 0;
        lcd.backlight();
      }
      delay(1000);
      break;
    case 0xFF02FD:
      Serial.println("OK   ");
      lcd.print("OK   ");
      if (current_screen == 2) {
        set_clock_stage++;
      }
      break;
    case 0xFF22DD:
      Serial.println("LEFT ");
      lcd.print("LEFT ");
      break;
    case 0xFF629D:
      Serial.println("UP   ");
      lcd.print("UP   ");
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
      lcd.print("RIGHT");
      break;
    case 0xFFA857:
      Serial.println("DOWN ");
      lcd.print("DOWN ");
      Serial.println("UP   ");
      lcd.print("UP   ");
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


void loop()
{

  alarmClose.checkAlarm();
  alarmOpen.checkAlarm();

  if (digitalRead(backlight_toggle)) {
    if (backlight_state == 0) {
      backlight_state = 1;
      lcd.noBacklight();
    } else {
      backlight_state = 0;
      lcd.backlight();
    }
    delay(1000);
  }

  if (current_screen == 0) { // default display
    lcd.setCursor(0, 0);
    lcd.print(rtc.formatTime());
    lcd.print("                ");
    lcd.setCursor(0, 1);

    if (alarmClose.getTimeLeft() > alarmOpen.getTimeLeft()) {
      lcd.print(alarmOpen.getTimeLeftFormat());
      lcd.print(" ");
      lcd.print(alarmOpen.getAction());
    } else {
      lcd.print(alarmClose.getTimeLeftFormat());
      lcd.print(" ");
      lcd.print(alarmClose.getAction());
    }
    lcd.print("                ");
  } else if (current_screen == 1) { // button clicked display
    screen_counter++;
    if (screen_counter == 50) {
      current_screen = 0;
      screen_counter = 0;
    }
    lcd.setCursor(0, 0);
    lcd.print("RTC: ");
    lcd.print(rtc.formatTime());
    lcd.print("                ");
  } else if (current_screen == 2) { // Set clock display
    if (screen_counter == 500) {
      current_screen = 0;
      screen_counter = 0;
    }
    screen_counter++;
    lcd.setCursor(0, 0);
    lcd.print("SET: ");
    printDigits(set_clock_hour);
    lcd.print(":");
    printDigits(set_clock_min);
    lcd.print(":");
    printDigits(set_clock_sec);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    if (set_clock_stage == 0) {
      lcd.print("HOUR U/D --> OK");
      lcd.setCursor(0, 5);
    } else if (set_clock_stage == 1) {
      lcd.print("MIN U/D --> OK");
      lcd.setCursor(0, 8);
    } else if (set_clock_stage == 2) {
      lcd.print("SEC U/D --> OK");
      lcd.setCursor(0, 11);
    } else if (set_clock_stage == 3) {
      rtc.setTime(set_clock_hour, set_clock_min, set_clock_sec);
      current_screen = 0;
      screen_counter = 0;
      set_clock_stage = 0;
    }
  }

  if (irrecv.decode(&results)) {
    lcd.setCursor(0, 1);
    lcd.print("                ");
    decode_value(results.value);
    screen_counter = 0;
    irrecv.resume();
  }
}
