#include <RV-3028-C7.h>
#include <LowPower.h>               // https://github.com/rocketscream/Low-Power
#include <U8g2_for_Adafruit_GFX.h>  // https://github.com/olikraus/U8g2_for_Adafruit_GFX
#include <u8g2_fonts.h>             // https://github.com/ZinggJM/GxEPD2
#include <GxEPD2_BW.h>              // including both doesn't use more code or ram
#include <GxEPD2_3C.h>              // including both doesn't use more code or ram

// select the display class and display driver class in the following file (new style):
#include "GxEPD2_display_selection.h"

#include <ez_switch_lib.h> // Library for the buttons

#define common_interrupt_pin 8 //Pin that the switches will link to via ez_switch. This pin will trigger the PCINT interupt
#define num_switches 2 //Number of switches that is linked to the trigger
Switches  my_switches(num_switches);

byte my_switch_data[][3] =
{
    button_switch,  4, circuit_C2, //circuit_C2 is a dirrectly attached circuit with no external resistor
    button_switch,  5, circuit_C2,
};

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;  // font constructor
RV3028 rtc;                       // create the RTC object

const uint8_t wakeUpPin(2);  // connect Arduino pin D2 to RTC's SQW pin.

//const uint8_t changeMin(4); // Button to change minutes
//const uint8_t changeHr(5); // Button to change hours
const uint8_t up(0); // Button to increase time
const uint8_t down(1); // Button to decrease time


//The below variables control what the date will be set to
int sec = 0;
int minute = 44;
int hour = 5;
int day = 3;
int date = 27;
int month = 12;
int year = 2023;


void setup() {
  int result;
  Serial.begin(115200);
  Wire.begin();
  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
    return;
    while (1)
      ;
  } else
    Serial.println("RTC online!");
  Serial.println();

  u8g2Fonts.begin(display);  // connect the u8g2


  // configure an interrupt on the falling edge from SQN pin
  pinMode(wakeUpPin, INPUT_PULLUP);

// time adjustment buttons with pullup resistor
  pinMode (up, INPUT_PULLUP);
  pinMode (down, INPUT_PULLUP);

// allows for the change time buttons to interput the loop
  PCICR |= B00000100; //turns on PCINT for pins in group d
  PCMSK2 |= B00110000; //Pins D4 and D5 will interupt
  rtc.enableTrickleCharge(TCR_3K);  //series resistor 3kOhm
  rtc.setTime(sec, minute, hour, day, date, month, year);  //USE THIS TO INITALLY SET TIME. Once set it needs to be commented out
  Serial.println("VOID SETUP = 1/2");
  displayDate();
}
/*
ISR (PCINT1_vect) {
  Serial.println("first stop in interupt");
  if (changeHr == 0) {
    uint8_t currentHour = rtc.getHours();
    Serial.println("within change hrs");
    do {
      rtc.setHours(currentHour++);
    }
    while (up == 0);
    do {
      rtc.setHours(currentHour - 1);
    }
    while (down == 0);

    }
  else if (changeMin == 0){
    return;
  }
}
*/

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

  rtc.updateTime();
  String timeString = rtc.stringTime();
  Serial.println(timeString);


  u8g2Fonts.setForegroundColor(GxEPD_BLACK);
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
  // Only numbers and symbols to save space.https://github.com/olikraus/u8g2/wiki/fntlist99#50-pixel-height
  u8g2Fonts.setFont(u8g2_font_logisoso50_tn);

  uint16_t x = 60;
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
      //u8g2Fonts.print(timeString); This one has seconds which does not work on e-paper because of low refresh rate
    }
  } while (display.nextPage());
  display.hibernate();
  Serial.println("DISPLAY TIME = finish");
}

void changeTime () {

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

}

void alarmIsr () {

}
