#include <RV-3028-C7.h>
#include <LowPower.h>               // https://github.com/rocketscream/Low-Power
#include <U8g2_for_Adafruit_GFX.h>  // https://github.com/olikraus/U8g2_for_Adafruit_GFX
#include <u8g2_fonts.h>             // https://github.com/ZinggJM/GxEPD2
#include <GxEPD2_BW.h>              // including both doesn't use more code or ram
#include <GxEPD2_3C.h>              // including both doesn't use more code or ram

// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection.h"

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;  // font constructor
RV3028 rtc;                       // create the RTC object

const uint8_t wakeUpPin(2);  // connect Arduino pin D2 to RTC's SQW pin.

const uint8_t changeFunc(4); // Button to change whether hours or minutes are changed and to turn off the change time mode
const uint8_t changeOn(5); // Button to turn on changing time mode
const uint8_t up(8); // Button to increase time
const uint8_t down(3); // Button to decrease time


//The below variables control what the date will be set to
int sec = 0;
int minute = 44;
int hour = 5;
int day = 3;
int date = 27;
int month = 12;
int year = 2023;

volatile int changeTime = 0;

void setup() {
  int result;
  Serial.begin(115200);
  Wire.begin();
  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
    while (1) 
    return;
    
  } else
    Serial.println("RTC online!");
  Serial.println();

  u8g2Fonts.begin(display);  // connect the u8g2


  // configure an interrupt on the falling edge from SQN pin
  pinMode(wakeUpPin, INPUT_PULLUP);

// time adjustment buttons with pullup resistor
  pinMode (changeOn, INPUT_PULLUP);
  pinMode (changeFunc, INPUT_PULLUP);
  pinMode (up, INPUT_PULLUP);
  pinMode (down, INPUT_PULLUP);

// allows for the change time buttons to interput the loop
  PCICR |= B00000100; //turns on PCINT for pins in group d
  PCMSK2 |= B00100000; //Pin D5 will interupt
  rtc.enableTrickleCharge(TCR_3K);  //series resistor 3kOhm
  rtc.setTime(sec, minute, hour, day, date, month, year);  //USE THIS TO INITALLY SET TIME. Once set it needs to be commented out
  Serial.println("VOID SETUP = 1/2");
  displayDate();
}

ISR (PCINT2_vect) {
  changeTime = 1;
}


// Displays the date in the bottom half of the screen
// and does a complete screen refresh
void displayDate() {
  Serial.println("DISPLAY DATE = start");

  display.init();

  display.setRotation(3);  //0 is 'portrait'

  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  u8g2Fonts.setFont(u8g2_font_logisoso20_tr);

  rtc.updateTime();

  String dateString = rtc.stringDate();  //rtc to output current date
  Serial.println(rtc.stringDate());

  uint16_t x = 70;
  uint16_t y = 110;  //bottom
  // covers bottom half
  display.setFullWindow();
  display.firstPage();
  do  // update the whole screen
  {
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(rtc.stringDate());
  } while (display.nextPage());
  display.hibernate();
  Serial.println("DISPLAY DATE. = finished");
}

// Displays the time in the top half of the screen, as a partial refresh
void displayTime() {
  Serial.println("DISPLAY TIME = start");
  Serial.println("1 ");
  rtc.updateTime();

  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  // Only numbers and symbols to save space.https://github.com/olikraus/u8g2/wiki/fntlist99#50-pixel-height
  u8g2Fonts.setFont(u8g2_font_logisoso50_tn);
  Serial.print("4 ");

  uint16_t x = 60;
  uint16_t y = 62;  //top half, depends on font
  Serial.print("5 ");

  display.setPartialWindow(0, 0, display.width(), display.height() / 2);
  display.firstPage();
  do  // Update the upper part of the screen
  {
      Serial.print("6 ");
    u8g2Fonts.setCursor(x, y);
    if (rtc.getHours() < 10) {
      u8g2Fonts.print(0);
      u8g2Fonts.print(rtc.getHours());
    } else {
      u8g2Fonts.print(rtc.getHours());
    }
    u8g2Fonts.print(":");
    if (rtc.getMinutes() < 10) {
      u8g2Fonts.print(0);
      u8g2Fonts.print(rtc.getMinutes());
    } else {
      u8g2Fonts.print(rtc.getMinutes());
      //u8g2Fonts.print(timeString); This one has seconds which does not work on e-paper because of low refresh rate
    }
  } while (display.nextPage());
  display.hibernate();
  Serial.println("DISPLAY TIME = finish");
}


void loop() {
  Serial.println("VOID LOOP = start");

  if (rtc.getMinutes() == 0) {
    // Refresh the display completely on the hour
    displayDate();
  }

  displayTime();

  Serial.println("VOID LOOP = before sleep");

  delay(500);  // if this isn't here the arduino seems to fall asleep before finishing the last line

  rtc.enablePeriodicUpdateInterrupt(0, 0);

  // Allow wake up pin to trigger on interrupt low.
  attachInterrupt(digitalPinToInterrupt(2), alarmIsr, FALLING);

  Serial.println("VOID LOOP = powering down");

  delay(500); // if this isn't here the arduino seems to fall asleep before finishing the last line

  // Power down
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // Wakes up here!
  Serial.println("VOID LOOP = woke up");

  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(2));

  while (changeTime == 1) {
    Serial.println("changetime");
    byte upState = digitalRead(up);
    byte downState = digitalRead(down);
    digitalRead(changeFunc);
    int changeFuncState = 0;

  if ( digitalRead(changeFunc) == HIGH ) { 
    changeFuncState++;
    delay (250);
  }

  if ( changeFuncState == 0 ) { // Changes the hours. Runs when changeFuncState == 0, therefore runs right after the ISR and is the default mode.
    if ( upState == HIGH ) {
      delay(250);
      Serial.println("hr, up");
      rtc.setHours(rtc.getHours() + 1);
      displayTime();
    }
    else if ( downState == HIGH ) {
      delay(250);
      Serial.println("hr, down");
      rtc.setHours(rtc.getHours() - 1);
      displayTime(); 
    }
  }

  else if ( changeFuncState == 1 ) { // Changes the minutes
    if ( upState == HIGH ) {
      delay(250);
      Serial.println("min, up");
      rtc.setHours(rtc.getMinutes() + 1);
      displayTime();
    }
    else if ( downState == HIGH ) {
      delay(250);
      Serial.println("min, down");
      rtc.setHours(rtc.getMinutes() - 1);
      displayTime(); 
    }

  }
  else if ( changeFuncState == 2 ) { //Turns off change time function of the clock
  changeFuncState = 0;
  changeTime = LOW;
  }

  } 
}



void alarmIsr () {

}
