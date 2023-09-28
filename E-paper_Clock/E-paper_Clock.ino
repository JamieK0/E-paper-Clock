// GxEPD2_HelloWorld.ino by Jean-Marc Zingg

// see GxEPD2_wiring_examples.h for wiring suggestions and examples
// if you use a different wiring, you need to adapt the constructor parameters!

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>

/*
  Setting and reading time from RV-3028-C7 Real Time Clock
  By: Constantin Koch
  Date: 7/31/2019
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting my work? Give me a star!
  https://github.com/constiko/RV-3028_C7-Arduino_Library

  This example shows how to set the time on the RTC to the compiler time or a custom time, and how to read the time.
  Open the serial monitor at 115200 baud.
*/
//E-paper
#include <GxEPD2_BW.h>
//#include <GxEPD2_3C.h>
//#include <GxEPD2_7C.h>
#include "FreeSansBoldSmall24pt7b.h"
#include "FreeSansSmall12pt7b.h"

// select the display class and display driver class in the following file (new style):
//#include "GxEPD2_display_selection_new_style.h"

// or select the display constructor line in one of the following files (old style):
#include "GxEPD2_display_selection.h"
//#include "GxEPD2_display_selection_added.h"

// alternately you can copy the constructor from GxEPD2_display_selection.h or GxEPD2_display_selection_added.h to here
// e.g. for Wemos D1 mini:
//GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=D8*/ SS, /*DC=D3*/ 0, /*RST=D4*/ 2, /*BUSY=D2*/ 4)); // GDEH0154D67

//RTC
#include <RV-3028-C7.h>


//#include <RV-3028-Clock.h>
RV3028 rtc;

//The below variables control what the date will be set to
int sec = 0;
int minute = 49;
int hour = 12;
int day = 4;
int date = 28;
int month = 9;
int year = 2023;
int refreshRate = 60000;

void setup() {
  //display.init(115200); // default 10ms reset pulse, e.g. for bare panels with DESPI-C02
  display.init(115200, true, 2, false);  // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.hibernate();
  while (!Serial)
    ;
  Serial.println("Read/Write Time - RTC Example");
  Wire.begin();
  /*if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
    while (1)
      ;
  } else
    Serial.println("RTC online!"); */
  delay(1000);
  //rtc.setTime(sec, minute, hour, day, date, month, year);
  rtc.enableTrickleCharge(TCR_3K);   //series resistor 3kOhm
}


/* String findCurrentDay() {

  char days[10][10] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };
  String currentDay = days[rtc.getWeekday()];
  return currentDay;
} */

/*
String AMPM() {
  String TimeAMPM;
  if (rtc.isPM() == true) {
    String TimeAMPM = "PM";
  } else {
    String TimeAMPM = "AM";
  }
  Serial.println ("Test" + TimeAMPM);
  return TimeAMPM;
}
*/

//const char currentTime[] = "";

void loop() {
  //RTC
  //PRINT TIME

  if (rtc.updateTime() == false)  //Updates the time variables from RTC
  {
    Serial.print("RTC failed to update");
  } else {
    String currentTime = rtc.stringTime();
    String currentDate = rtc.stringDate();

    Serial.println(currentTime + "     \'s\' = set time     \'1\' = 12 hours format     \'2\' = 24 hours format");

    //Test custom hh:mm
    //Serial.println(rtc.getHours(), rtc.getMinutes());


    //E-paper Display text
    display.setRotation(3);
    display.setFont(&FreeSansBoldSmall24pt7b);
    display.setTextColor(GxEPD_BLACK);
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(currentTime, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center the bounding box by transposition of the origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;
    display.setFullWindow();
    display.firstPage();
    do {
      //findCurrentDay();  //Finds the current day of the week from the RTC clock. RTC clock outputs day of week in 8-bit int
      //String currentDay = findCurrentDay();
      //AMPM (); //Checks if it is AM or PM
      //String 12hr = AMPM();
      display.fillScreen(GxEPD_WHITE);
      display.setCursor(x, 60);
      display.print(rtc.getHours());
      display.print(" : ");
      display.print(rtc.getMinutes());
      //display.print(AMPM());
      display.setCursor(20, 120);  //Date and Day start
      display.setFont(&FreeSansSmall12pt7b);
      display.print(currentDate);
      display.setCursor(20, 120);
      //display.print(currentDay);
      Serial.print(rtc.getMinutes());
    } while (display.nextPage());
    delay(refreshRate);
  }
  //SET TIME?
  if (Serial.available()) {
    switch (Serial.read()) {
      case 's':
        //Use the time from the Arduino compiler (build time) to set the RTC
        //Keep in mind that Arduino does not get the new compiler time every time it compiles. to ensure the proper time is loaded, open up a fresh version of the IDE and load the sketch.
        if (rtc.setToCompilerTime() == false) {
          Serial.println("Something went wrong setting the time");
        }
        //Uncomment the below code to set the RTC to your own time
        if (rtc.setTime(sec, minute, hour, day, date, month, year) == false) {
          Serial.println("Something went wrong setting the time");
        }
        break;
      case '1':
        rtc.set12Hour();
        break;

      case '2':
        rtc.set24Hour();
        break;
    }
  }
};