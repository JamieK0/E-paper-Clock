#include <RV-3028-C7.h>             // RTC Library
#include <LowPower.h>               // Reduces power consumption https://github.com/rocketscream/Low-Power
#include <U8g2_for_Adafruit_GFX.h>  // https://github.com/olikraus/U8g2_for_Adafruit_GFX
#include <GxEPD2_BW.h>              // including both doesn't use more code or ram

// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection.h"

//The below variables control what the date will be set to. Uncomment the line bellow in setup that sets the rtc to the time here. After compiling, the line needs to be commented out again so that the Arduino does not set the time to this time every time it loses power
int sec = 0;
int minute = 52;
int hour = 12;
int day = 5;
int date = 29;
int month = 12;
int year = 2023;

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;  // font constructor
RV3028 rtc;                       // create the RTC object

const uint8_t wakeUpPin(2);  // connect Arduino pin D2 to RTC's SQW pin. Wakes up the Arduino each minute from sleep mode.

const uint8_t changeFunc(4);  // Button to change function from changing hours to changing minutes to exit changing time mode
const uint8_t changeOn(5);    // Button to turn on changing time mode, this button causes the interrupt. Do not change this pin to any other pin unless the PCINT paramaters are also changed below.
const uint8_t up(8);          // Button to increase time
const uint8_t down(3);        // Button to decrease time

volatile int changeTime = 0; //Integer that gets changed when the interrupt occurs
int changeFuncState = 0;  //Function button set to 0 initally

void setup() {
  Serial.begin(115200);
  Wire.begin();
  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
    while (1)
      return;
  } else
    Serial.println("RTC online!");
  Serial.println();

  u8g2Fonts.begin(display);  // connect the u8g2 display

  // configures interupt pin
  pinMode(wakeUpPin, INPUT_PULLUP);

  // time adjustment buttons with pullup resistor
  pinMode(changeOn, INPUT_PULLUP);
  pinMode(changeFunc, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);

  // allows for the change time buttons to interput the loop
  PCICR |= B00000100;                                      //turns on PCINT for pins in group D
  PCMSK2 |= B00100000;                                     //Pin D5 will cause interupt
  rtc.enableTrickleCharge(TCR_3K);                         //series resistor 3kOhm
  //rtc.setTime(sec, minute, hour, day, date, month, year);  //USE THIS TO INITALLY SET TIME. Once set it needs to be commented out so that it doesn't get set to this time every restart
  displayDate();
}

ISR(PCINT2_vect) {
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

  uint16_t x = 73;
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
  Serial.println("DISPLAY DATE = finished");
}

// Displays the time in the top half of the screen, as a partial refresh
void displayTime() {
  Serial.println("DISPLAY TIME = start");
  rtc.updateTime();

  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  // Only numbers and symbols to save space.https://github.com/olikraus/u8g2/wiki/fntlist99#50-pixel-height
  u8g2Fonts.setFont(u8g2_font_logisoso50_tn);

  uint16_t x = 70;
  uint16_t y = 62;  //top half, depends on font

  display.setPartialWindow(0, 0, display.width(), display.height() / 2);
  display.firstPage();
  do  // Update the upper part of the screen
  {
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
    }
  } while (display.nextPage());
  display.hibernate();
  Serial.println("DISPLAY TIME = finish");
}


void loop() {
  Serial.println("LOOP = start");

  if (rtc.getMinutes() == 0) {
    // Refresh the display completely on the hour
    displayDate();
  }

  displayTime();

  Serial.println("LOOP = before sleep");

  delay(500);

  rtc.enablePeriodicUpdateInterrupt(0, 0);

  // Allow wake up pin to trigger on interrupt low.
  attachInterrupt(digitalPinToInterrupt(2), alarmIsr, FALLING);

  Serial.println("VOID LOOP = powering down");

  delay(500);  // if this isn't here the arduino seems to fall asleep before finishing the last line

  // Power down
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // Wakes up here!
  Serial.println("VOID LOOP = woke up");

  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(2));

// Change time code
  while (changeTime == 1) {
    Serial.println("changetime");
    byte upState = digitalRead(up);
    byte downState = digitalRead(down);
    digitalRead(changeFunc);


    if (digitalRead(changeFunc) == LOW) {
      changeFuncState = changeFuncState + 1;
      Serial.println(changeFuncState);
      delay(250);
    }

    if (changeFuncState == 0) {  // Changes the hours. Runs when changeFuncState == 0, therefore runs right after the ISR and is the default mode.
      if (upState == LOW) {
        delay(250);
        Serial.println("hr, up");
        rtc.setHours(rtc.getHours() + 1);
        displayTime();
      } else if (downState == LOW) {
        delay(250);
        Serial.println("hr, down");
        rtc.setHours(rtc.getHours() - 1);
        displayTime();
      }
    }

    else if (changeFuncState == 1) {  // Changes the minutes
      if (upState == LOW) {
        delay(250);
        Serial.println("min, up");
        rtc.setMinutes(rtc.getMinutes() + 1);
        displayTime();
      } else if (downState == LOW) {
        delay(250);
        Serial.println("min, down");
        rtc.setMinutes(rtc.getMinutes() - 1);
        displayTime();
      }

    } else if (changeFuncState == 2) {  //Turns off change time function of the clock
      changeFuncState = 0;
      changeTime = LOW;
      displayDate();  //Fully refreshes the display
    }
  }
}



void alarmIsr() {
}
