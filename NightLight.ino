#include "RTClib.h"
#include <ezButton.h>

#define LED1_PIN 16
#define LED2_PIN 17
#define BTN_PIN 14

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// SET START/END TIMES HERE
// Normal hours
const int START_HOUR = 19;
const int START_MIN = 00;
const int END_HOUR = 1;
const int END_MIN = 30;
// Weekend
const int WEEKEND_END_HOUR = 2;
const int WEEKEND_END_MIN = 15;
// first warning time
const int FIRST_WARN_MIN = 5;
// final warning time
const unsigned int FINAL_WARN_MIN = 1;

bool switched_on = false;

TimeSpan first_warning_time = TimeSpan(0, 0, FIRST_WARN_MIN, 0);
TimeSpan final_warning_time = TimeSpan(0, 0, FINAL_WARN_MIN, 0);

RTC_DS1307 rtc;
ezButton button(BTN_PIN);  // create ezButton object that attach to pin 7;

bool LEDs_on = false;

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT);
  button.setDebounceTime(50); // set debounce time to 50 milliseconds
  
  Serial.begin(9600);
  delay(1500); // wait for console opening
    
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC lost power, lets set the time!");
  }
  
    // Comment out below lines once you set the date & time.
    // Following line sets the RTC to the date & time this sketch was compiled
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  print_time();
  Serial.println("Night light boot up complete.");
  //blink_first_warning();
  //delay(1000);
  //blink_final_warning();
  //fade_on();
}

void print_time(){
    DateTime now = rtc.now();
    
    Serial.println("Current Date & Time: ");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
}

void fade_on(){
  for(unsigned int i = 0; i < 3000; i++){
    analogWrite(LED1_PIN,(i / 1000.0) * 255);
    analogWrite(LED2_PIN,(i / 1000.0) * 255);
    delay(1);
  }
}

void fade_off(){
  for(unsigned int i = 3000; i > 0; i--){
    analogWrite(LED1_PIN,(i / 1000.0) * 255);
    analogWrite(LED2_PIN,(i / 1000.0) * 255);
    delay(1);
  }
}

void blink_first_warning(){
  for(unsigned int i = 0; i < 6; i++){
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    delay(250);
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    delay(1000);
  }
}

void blink_final_warning(){
  for(unsigned int i = 0; i < 15; i++){
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    delay(150);
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    delay(150);
  }
}

void loop() {
    button.loop(); // MUST call the loop() function first

    DateTime now = rtc.now();
    DateTime first_warning = now + first_warning_time;
    DateTime final_warning = now + final_warning_time;

    // Check time
    unsigned int hour = now.hour();
    unsigned int min = now.minute();
    unsigned int fir_hour = first_warning.hour();
    unsigned int fir_min = first_warning.minute();
    unsigned int fir_sec = first_warning.second();
    unsigned int fin_hour = final_warning.hour();
    unsigned int fin_min = final_warning.minute();
    unsigned int fin_sec = final_warning.second();
    unsigned int dow = now.dayOfTheWeek();
    bool weekend = false;
    bool switching_on = false;
    bool switching_off = false;

    
    // TODO: better weekend calculation based on current time (eg. if before 12, after 12)
    if(dow == 0 || dow == 6){
      weekend = true;
    }

    DateTime off_time;
    DateTime on_time = DateTime(now.year(), now.month(), now.day(), START_HOUR, START_MIN, 0);
    if(hour < 9){
      on_time = on_time - TimeSpan(1, 0, 0, 0);
    } 
    if(weekend){
      off_time = DateTime(now.year(), now.month(), now.day(), WEEKEND_END_HOUR, WEEKEND_END_MIN, 0);
      if(WEEKEND_END_HOUR >= 0 && WEEKEND_END_HOUR < 12){
        off_time = off_time + TimeSpan(1, 0, 0, 0);
      }
    } else {
      off_time = DateTime(now.year(), now.month(), now.day(), END_HOUR, END_MIN, 0);
      if(END_HOUR >= 0 && END_HOUR < 12){
        off_time = off_time + TimeSpan(1, 0, 0, 0);
      }
    }
    
    // Same start hour regardless of weekend
    TimeSpan start_diff = now - on_time;
    TimeSpan end_diff = off_time - now;
    Serial.println(start_diff.totalseconds());
    Serial.println(end_diff.totalseconds());
    if(start_diff.totalseconds() >= 0 && end_diff.totalseconds() > 0){
      switching_on = true;
    }

    // Different end times depending if it is weekend or not
    if(!weekend){
      // First warning time
      if(fir_hour == END_HOUR && fir_min == END_MIN && fir_sec < 5){
        blink_first_warning();
      } /* Final warning time */
      else if(fin_hour == END_HOUR && fin_min == END_MIN && fin_sec < 5){
        blink_final_warning();
      } /* Actial end time */
      else if(hour == END_HOUR && min == END_MIN){
        switching_off = true;
      }
    } /* Special weekend end times */ 
    else if (weekend) {
      if(fir_hour == WEEKEND_END_HOUR && fir_min == WEEKEND_END_MIN && fir_sec < 5){
        blink_first_warning();
      } /* Final warning time */
      else if(fin_hour == WEEKEND_END_HOUR && fin_min == WEEKEND_END_MIN && fin_sec < 5){
        blink_final_warning();
      } /* Actial end time */
      else if(hour == WEEKEND_END_HOUR && min == WEEKEND_END_MIN){
        switching_off = true;
      }
    }
    // Fade on and off
    if(!LEDs_on && switching_on){
      fade_on();
      LEDs_on = true;
      switched_on = true;
    } else if (LEDs_on && switching_off){
      fade_off();
      LEDs_on = false;
      switched_on = false;
    }
    
    if(button.isPressed()) {

      switched_on = !switched_on;   
  
      // control LED arccoding to the toggleed sate
      digitalWrite(LED1_PIN, switched_on); 
      digitalWrite(LED2_PIN, switched_on);
    }
}
