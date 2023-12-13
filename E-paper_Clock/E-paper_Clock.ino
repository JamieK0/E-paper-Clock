// Simple Clock with date on epaper screen with Arduino
// www.henryleach.com

#include <RV-3028-C7.h>
#include <LowPower.h>  // https://github.com/rocketscream/Low-Power
#include <U8g2_for_Adafruit_GFX.h> // https://github.com/olikraus/U8g2_for_Adafruit_GFX
#include <u8g2_fonts.h>
// https://github.com/ZinggJM/GxEPD2
#include <GxEPD2_BW.h> // including both doesn't use more code or ram
#include <GxEPD2_3C.h> // including both doesn't use more code or ram
// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection.h"

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;  // font constructor
RV3028 rtc;  // create the RTC object

const uint8_t wakeUpPin(2); // connect Arduino pin D2 to RTC's SQW pin.
const uint8_t dstPin(4); // connect to GND to add 1 hour as DST.

//The below variables control what the date will be set to
int sec = 0;
int minute = 2;
int hour = 5;
int day = 1;
int date = 9;
int month = 10;
int year = 2023;


void setup()
{

  Serial.begin(115200);
    Wire.begin();
  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
    while (1);
  }
  else
    Serial.println("RTC online!");
  Serial.println();

  u8g2Fonts.begin(display); // connect the u8g2


  // configure an interrupt on the falling edge from SQN pin
  pinMode(wakeUpPin, INPUT_PULLUP);
  pinMode(dstPin, INPUT_PULLUP);
  rtc.enableTrickleCharge(TCR_3K);   //series resistor 3kOhm
  rtc.setTime(sec, minute, hour, day, date, month, year);  //USE THIS TO INITALLY SET TIME
  Serial.println("VOID SETUP = 1/2");
  getDstTime();
  displayDate();
  
}

// Displays the date in the bottom half of the screen
// and does a complete screen refresh
void displayDate()
{
    Serial.println("DISPLAY DATE = start");

  display.init();
  
  display.setRotation(3); //0 is 'portrait'

  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  u8g2Fonts.setFont(u8g2_font_logisoso20_tr);

  rtc.updateTime();

  String dateString = rtc.stringDate(); //rtc to output current date
      Serial.println(rtc.stringDate());

  uint16_t x = 70;
  uint16_t y = 110; //bottom
  // covers bottom half
  display.setFullWindow();
  display.firstPage();
  do // update the whole screen
  {
    u8g2Fonts.setCursor(x, y);
    u8g2Fonts.print(dateString); 
  }
  while (display.nextPage());
  display.hibernate();
    Serial.println("DISPLAY DATE. = finished");

}

// Displays the time in the top half of the screen, as a partial refresh
void displayTime()
{
      Serial.println("DISPLAY TIME = start");
    
    rtc.updateTime();
    String timeString = rtc.stringTime();


  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  // Only numbers and symbols to save space.https://github.com/olikraus/u8g2/wiki/fntlist99#50-pixel-height
  u8g2Fonts.setFont(u8g2_font_logisoso50_tn);
  
  uint16_t x = 60;
  uint16_t y = 62; //top half, depends on font

  display.setPartialWindow(0, 0, display.width(), display.height() / 2);
  display.firstPage();
  do // Update the upper part of the screen
  {
    u8g2Fonts.setCursor(x, y);
    if (rtc.getHours() < 10) {
      u8g2Fonts.print(0);
      u8g2Fonts.print(rtc.getHours());
      }
    else {
      u8g2Fonts.print(rtc.getHours());  
    }
    u8g2Fonts.print(":");
    if (rtc.getMinutes() < 10) {
      u8g2Fonts.print(0);
      u8g2Fonts.print(rtc.getMinutes());
    }
    else {
    u8g2Fonts.print(rtc.getMinutes());
    //u8g2Fonts.print(timeString); This one has seconds which does not work on e-paper because of low refresh rate
    }
  }
  while (display.nextPage());
  display.hibernate();
      Serial.println("DISPLAY TIME = finish");

}

void loop()
{
  Serial.println("VOID LOOP = start");

  getDstTime();

  if (rtc.getMinutes() == 0){
    // Refresh the display completely
    // every hour, hopefully reduces ghosting and burn in
    // also updates the date at midnight
        displayDate();

  }
// IM GOING TO TAKE DISPLAYDATE OUT OF THE LOOP SO THAT I CAN SEE IT REFRESH FASTER FOR TESTING

  displayTime();

  Serial.println("VOID LOOP = before sleep");

  delay(500); // if this isn't here the arduino seems to fall asleep before finishing the last job

  setAlarm();


  // Allow wake up pin to trigger on interrupt low.
  attachInterrupt(digitalPinToInterrupt(2), alarmIsr, FALLING);

  // Power down
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // Wakes up here!
  Serial.println("VOID LOOP = woke up");

  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(2));
/*
  if (myRTC.alarm(DS3232RTC::ALARM_1)) {
    //also clears alarm flag
  }
*/
}

void setAlarm() {
	rtc.enablePeriodicUpdateInterrupt(0, 0);
  // setAlarm(alarmType, seconds, minutes, hours, daydate)
  /* myRTC.setAlarm(DS3232RTC::ALM1_MATCH_SECONDS, 0, 0, 0, 1); // starts on next whole minute
  myRTC.alarm(DS3232RTC::ALARM_1);
  myRTC.alarmInterrupt(DS3232RTC::ALARM_1, true); */
}

void getDstTime() {
  // Get the time, and adjust for DST if pin D4 is connected to GND
  /*t = myRTC.get(); // get the time from the RTC

  if (digitalRead(dstPin) == LOW){
    t+=3600; // Add 1 hour in seconds
  }
  */
}

void alarmIsr()
{
  // Need a function for the interrupt, but doesn't have to do anything.
}
