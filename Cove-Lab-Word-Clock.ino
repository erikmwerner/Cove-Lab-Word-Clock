/* Cove Clock

    based off Javelin Word Clock
    https://github.com/nitrohawk/Javelin-Word-Clock/blob/master/Word_Clock.ino

    Written by Erik Werner and Monica Kim
    2017 12 14

    Version 1.2
    Updated 2015 12 15
    Fixes brightness issue during rainbow
    Adds date setting via serial port
 *  */


// add in libraries
// for the LED grid
#include <Adafruit_NeoPixel.h>

// for the RTC clock https://learn.adafruit.com/adafruit-ds3231-precision-rtc-breakout/arduino-usage
#include "RTClib.h"
#include <Wire.h> // i2c communication

// define some constants for later use
#define RGBLEDPIN 6
#define N_LEDS 117 // 13 x 9

// Assuming 5W TDP and 0.3W power draw per LED at full brightness:
// 50 LEDS can be on at once at 33% brightness (85/255 PWM)
// WARNING: do not set brightness above 100 or 150 and leave the clock on
// for sustained periods of time!
#define BRIGHTNESS 100 // 50 to 100 seems to work well 
#define FADE_TIME 100

#define TEST_MODE true
#define TEST_SPEED 50

// Define some variables for the clock
String strTime = ""; // used to detect if word time has changed
int intTimeUpdated = 0; // used to tell if we need to turn on the brightness again when words change

Adafruit_NeoPixel grid = Adafruit_NeoPixel(N_LEDS, RGBLEDPIN, NEO_GRB + NEO_KHZ800);
RTC_DS3231 rtc;
DateTime now;

// define colors by RGB values
// to add colors, define them here so they can be reused
uint32_t colorWhite = grid.Color(BRIGHTNESS, BRIGHTNESS, BRIGHTNESS);
uint32_t colorBlack = grid.Color(0, 0, 0);
uint32_t colorRed = grid.Color(BRIGHTNESS, 0, 0);
uint32_t colorGreen = grid.Color(0, BRIGHTNESS, 0);
uint32_t colorBlue = grid.Color(0, 0, BRIGHTNESS);

// the words to light up
// each word is an array containing the
// indices of the LEDS that should light up
// where 0 is the first LED and 116 is the last LED

// TK i'm guessing the -1 is to tell the code that it's
// the last element in the array, since the arrays
// are all different lengths
int arrHELLO[] = {116, 115, 114, 113, 112, -1};
int arrRICHARD[] = {110, 109, 108, 107, 106, 105, 104, -1};
int arrIT[] = {91, 92, -1};
int arrIS[] = {94, 95, -1};
int arrMTEN[] = {97, 98, 99, -1};
int arrHALF[] = {100, 101, 102, 103, -1};
int arrQUARTER[] = {90, 89, 88, 87, 86, 85, 84, -1};
int arrTWENTY[] = {83, 82, 81, 80, 79, 78, -1};
int arrMFIVE[] = {65, 66, 67, 68, -1};
int arrMINUTES[] = {70, 71, 72, 73, 74, 75, 76, -1};
int arrPAST[] = {64, 63, 62, 61, -1};
int arrTO[] = {60, 59, -1};
int arrONE[] = {57, 56, 55, -1};
int arrTWO[] = {54, 53, 52, -1};
int arrTHREE[] = {39, 40, 41, 42, 43, -1};
int arrFOUR[] = {44, 45, 46, 47, -1};
int arrFIVE[] = {48, 49, 50, 51, -1};
int arrSIX[] = {38, 37, 36, -1};
int arrSEVEN[] = {35, 34, 33, 32, 31, -1};
int arrEIGHT[] = {30, 29, 28, 27, 26 - 1};
int arrNINE[] = {13, 14, 15, 16, -1};
int arrTEN[] = {17, 18, 19, -1};
int arrELEVEN[] = {20, 21, 22, 23, 24, 25, -1};
int arrTWELVE[] = {12, 11, 10, 9, 8, 7, -1};
int arrOCLOCK[] = {5, 4, 3, 2, 1, 0, -1};

void setup() {
  Serial.begin(9600);

  // check that the RTC is here and working
  if (! rtc.begin()) {
    Serial.println("Error: could not find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println(F("Error: RTC lost power. Please set the time"));
    Serial.println(F("Clock time resetting to software upload time..."));
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    printInstructions();
  }
  Serial.flush();

  // need to start up the LED strip library
  grid.begin();
  grid.show();
  grid.setBrightness(BRIGHTNESS);

  // print instructions for programming the time
  printInstructions();

  // have it do a rainbow for fun

  rainbow(10);

  colorWipe(colorBlack, 0);

  // start the clock
  digitalClockDisplay();
}

void loop() {
  // if there is a serial connection check to see if the time has been changed
  if ( Serial.available() ) {
    readSerialMessage();
    Serial.flush();
  }

  //update current time
  if(TEST_MODE) {
    now = now + TimeSpan(TEST_SPEED);
  }
  else {
    now = rtc.now();
  }

  // show the time
  displayTime();
}

void readSerialMessage() {
  interrupts();
  delay(1000);
  Serial.println("Reading serial data...");
  delay(1000);
  String input = Serial.readString();
  
  Serial.println("You entered: ");
  Serial.println(input);
  
  if(input.charAt(0) == 'h' || input.charAt(0) == 'H') {
    Serial.println("Hour: ");
    int h = input.substring(1).toInt();
    Serial.println(h);
    DateTime newDate(now.year(), now.month(), now.day(), h, now.minute(), now.second());
    rtc.adjust(newDate);
  }
  else if(input.charAt(0) == 'm' || input.charAt(0) == 'M') {
    Serial.println("Minute: ");
    int m = input.substring(1).toInt();
    Serial.println(m);
    DateTime newDate(now.year(), now.month(), now.day(), now.hour(), m, now.second());
    rtc.adjust(newDate);
  }
  else if(input.charAt(0) == 's' || input.charAt(0) == 'S') {
    Serial.println("Second: ");
    int s = input.substring(1).toInt();
    Serial.println(s);
    DateTime newDate(now.year(), now.month(), now.day(), now.hour(), now.minute(), s);
    rtc.adjust(newDate);
  }
  else if(input.charAt(0) == 't' || input.charAt(0) == 'T') {
    rainbow(10);
  }
  else if(input.charAt(0) == 'w' || input.charAt(0) == 'W') {
    colorWipe(colorWhite, 0);
    delay(5000);
  }
  else if(input.charAt(0) == 'b' || input.charAt(0) == 'B') {
    colorWipe(colorBlack, 0);
    delay(5000);
  }

  colorWipe(colorBlack, 0);
  String strTime = "";
}


// digital clock display of the time
void digitalClockDisplay() {
  Serial.print(now.hour());
  printDigits(now.minute());
  printDigits(now.second());
  Serial.print(" ");
  Serial.print(now.year());
  Serial.print("-");
  Serial.print(now.month());
  Serial.print("-");
  Serial.print(now.day());
  Serial.println();
}

void displayTime() {
  String strCurrentTime; // build the current time

  // optional: blank the display
  // use this if LEDS do not "turn off"
  // after their time has elapsed
  // colorWipe(colorBlack, 0);

  // always display these thigns
  // (logo or name and "it is" LEDs)
  paintWord(arrHELLO, colorBlue);
  paintWord(arrRICHARD, colorGreen);
  paintWord(arrIT, colorWhite);

  // if the time IS one of the following IS is green
  if ((now.minute() == 5)
      | (now.minute() == 10)
      | (now.minute() == 15)
      | (now.minute() == 20)
      | (now.minute() == 25)
      | (now.minute() == 30)
      | (now.minute() == 35)
      | (now.minute() == 40)
      | (now.minute() == 45)
      | (now.minute() == 50)
      | (now.minute() == 55)) {
    paintWord(arrIS, colorGreen);
  } else {
    paintWord(arrIS, colorWhite);
  }

  // now we display the appropriate minute counter
  if ((now.minute() > 4) && (now.minute() < 10)) {
    // FIVE MINUTES
    strCurrentTime = "five ";
    paintWord(arrMFIVE, colorWhite);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorBlack);
  }
  if ((now.minute() > 9) && (now.minute() < 15)) {
    //TEN MINUTES;
    strCurrentTime = "ten ";
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrMTEN, colorWhite);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorBlack);
  }
  if ((now.minute() > 14) && (now.minute() < 20)) {
    // QUARTER
    strCurrentTime = "quarter ";
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrQUARTER, colorWhite);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorBlack);
  }
  if ((now.minute() > 19) && (now.minute() < 25)) {
    //TWENTY MINUTES
    strCurrentTime = "twenty ";
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorWhite);
  }
  if ((now.minute() > 24) && (now.minute() < 30)) {
    //TWENTY FIVE
    strCurrentTime = "twenty five ";
    paintWord(arrMFIVE, colorWhite);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorWhite);
  }
  if ((now.minute() > 29) && (now.minute() < 35)) {
    strCurrentTime = "half ";
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrHALF, colorWhite);
    paintWord(arrTWENTY, colorBlack);
  }
  if ((now.minute() > 34) && (now.minute() < 40)) {
    //TWENTY FIVE
    strCurrentTime = "twenty five ";
    paintWord(arrMFIVE, colorWhite);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorWhite);

  }
  if ((now.minute() > 39) && (now.minute() < 45)) {
    strCurrentTime = "twenty ";
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorWhite);
  }
  if ((now.minute() > 44) && (now.minute() < 50)) {
    strCurrentTime = "quarter ";
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrQUARTER, colorWhite);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorBlack);
  }
  if ((now.minute() > 49) && (now.minute() < 55)) {
    strCurrentTime = "ten ";
    paintWord(arrMFIVE, colorBlack);
    paintWord(arrMTEN, colorWhite);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorBlack);
  }
  if (now.minute() > 54) {
    strCurrentTime = "five ";
    paintWord(arrMFIVE, colorWhite);
    paintWord(arrMTEN, colorBlack);
    paintWord(arrQUARTER, colorBlack);
    paintWord(arrHALF, colorBlack);
    paintWord(arrTWENTY, colorBlack);
  }
  if ((now.minute() < 5)) {
    switch (now.hour()) {
      case 1:
      case 13:
        strCurrentTime = strCurrentTime + "one ";
        paintWord(arrONE, colorWhite);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 2:
      case 14:
        strCurrentTime = strCurrentTime + "two ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorWhite);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 3:
      case 15:
        strCurrentTime = strCurrentTime + "three ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorWhite);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 4:
      case 16:
        strCurrentTime = strCurrentTime + "four ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorWhite);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 5:
      case 17:
        strCurrentTime = strCurrentTime + "five ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorWhite);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 6:
      case 18:
        strCurrentTime = strCurrentTime + "six ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorWhite);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 7:
      case 19:
        strCurrentTime = strCurrentTime + "seven ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorWhite);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 8:
      case 20:
        strCurrentTime = strCurrentTime + "eight ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorWhite);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 9:
      case 21:
        strCurrentTime = strCurrentTime + "nine ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorWhite);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 10:
      case 22:
        strCurrentTime = strCurrentTime + "ten ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorWhite);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 11:
      case 23:
        strCurrentTime = strCurrentTime + "eleven ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorWhite);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 0:
      case 12:
      case 24:
        strCurrentTime = strCurrentTime + "twelve ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorWhite);
        break;
    }
    strCurrentTime = strCurrentTime + "oclock ";
    paintWord(arrPAST, colorBlack);
    paintWord(arrOCLOCK, colorWhite);
    paintWord(arrTO, colorBlack);
  } else if ((now.minute() < 35) && (now.minute() > 4)) {
    strCurrentTime = strCurrentTime + "past ";
    paintWord(arrPAST, colorWhite);
    paintWord(arrOCLOCK, colorBlack);
    paintWord(arrTO, colorBlack);
    switch (now.hour()) {
      case 1:
      case 13:
        strCurrentTime = strCurrentTime + "one ";
        paintWord(arrONE, colorWhite);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 2:
      case 14:
        strCurrentTime = strCurrentTime + "two ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorWhite);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 3:
      case 15:
        strCurrentTime = strCurrentTime + "three ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorWhite);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 4:
      case 16:
        strCurrentTime = strCurrentTime + "four ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorWhite);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 5:
      case 17:
        strCurrentTime = strCurrentTime + "five ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorWhite);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 6:
      case 18:
        strCurrentTime = strCurrentTime + "six ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorWhite);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 7:
      case 19:
        strCurrentTime = strCurrentTime + "seven ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorWhite);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 8:
      case 20:
        strCurrentTime = strCurrentTime + "eight ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorWhite);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 9:
      case 21:
        strCurrentTime = strCurrentTime + "nine ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorWhite);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 10:
      case 22:
        strCurrentTime = strCurrentTime + "ten ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorWhite);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 11:
      case 23:
        strCurrentTime = strCurrentTime + "eleven ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorWhite);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 0:
      case 12:
      case 24:
        strCurrentTime = strCurrentTime + "twelve ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorWhite);
        break;
    }
  } else {
    // if we are greater than 34 minutes past the hour then display
    // the next hour, as we will be displaying a 'to' sign
    strCurrentTime = strCurrentTime + "to ";
    paintWord(arrPAST, colorBlack);
    paintWord(arrOCLOCK, colorBlack);
    paintWord(arrTO, colorWhite);
    switch (now.hour()) {
      case 1:
      case 13:
        strCurrentTime = strCurrentTime + "two ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorWhite);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 14:
      case 2:
        strCurrentTime = strCurrentTime + "three ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorWhite);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 15:
      case 3:
        strCurrentTime = strCurrentTime + "four ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorWhite);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 4:
      case 16:
        strCurrentTime = strCurrentTime + "five ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorWhite);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 5:
      case 17:
        strCurrentTime = strCurrentTime + "six ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorWhite);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 6:
      case 18:
        strCurrentTime = strCurrentTime + "seven ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorWhite);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 7:
      case 19:
        strCurrentTime = strCurrentTime + "eight ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorWhite);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 8:
      case 20:
        strCurrentTime = strCurrentTime + "nine ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorWhite);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 9:
      case 21:
        strCurrentTime = strCurrentTime + "ten ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorWhite);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 10:
      case 22:
        strCurrentTime = strCurrentTime + "eleven ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorWhite);
        paintWord(arrTWELVE, colorBlack);
        break;
      case 11:
      case 23:
        strCurrentTime = strCurrentTime + "twelve ";
        paintWord(arrONE, colorBlack);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorWhite);
        break;
      case 0:
      case 12:
      case 24:
        strCurrentTime = strCurrentTime + "one ";
        paintWord(arrONE, colorWhite);
        paintWord(arrTWO, colorBlack);
        paintWord(arrTHREE, colorBlack);
        paintWord(arrFOUR, colorBlack);
        paintWord(arrFIVE, colorBlack);
        paintWord(arrSIX, colorBlack);
        paintWord(arrSEVEN, colorBlack);
        paintWord(arrEIGHT, colorBlack);
        paintWord(arrNINE, colorBlack);
        paintWord(arrTEN, colorBlack);
        paintWord(arrELEVEN, colorBlack);
        paintWord(arrTWELVE, colorBlack);
        break;
    }
  }

  // if the time has changed...
  if (strCurrentTime != strTime) {
    // update the LEDs
    digitalClockDisplay();
    strTime = strCurrentTime;
    if (strTime == "") {
      fadeIn(20);
    }
  }
  else {
    // the time has not changed since the last loop
    // there is nothing to do...repeat the loop
    // grid.show();
  }
}


/*---------------------------------
   The rest of these functions are "helper"
   functions

   --------------------------------- */
void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void rainbow(uint8_t wait) {
  //secret rainbow mode
  uint16_t i, j;
  for (j = 0; j < 256; j++) {
    for (i = 0; i < grid.numPixels(); i++) {
      grid.setPixelColor(i, Wheel((i + j) & 255));
    }
    grid.show();
    delay(wait);
  }
}

static void chase(uint32_t color, uint8_t wait) {
  for (uint16_t i = 0; i < grid.numPixels() + 4; i++) {
    grid.setPixelColor(i  , color); // Draw new pixel
    grid.setPixelColor(i - 4, 0); // Erase pixel a few steps back
    grid.show();
    delay(wait);
  }
}

void fadeOut(int time) {
  for (int i = FADE_TIME; i > 0; --i) {
    grid.setBrightness(i);
    grid.show();
    delay(time);
  }
}

void fadeIn(int time) {
  for (int i = 1; i < FADE_TIME; ++i) {
    grid.setBrightness(i);
    grid.show();
    delay(time);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t color, uint8_t wait) {
  for (uint16_t i = 0; i < grid.numPixels(); i++) {
    grid.setPixelColor(i, color);
  }
  grid.show();
  delay(wait);
}

// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return grid.Color((WheelPos * 3) * BRIGHTNESS, (255 - WheelPos * 3) * BRIGHTNESS / 255.0, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return grid.Color((255 - WheelPos * 3) * BRIGHTNESS / 255.0, 0, (WheelPos * 3) * BRIGHTNESS / 255.0);
  } else {
    WheelPos -= 170;
    return grid.Color(0, (WheelPos * 3) * BRIGHTNESS / 255.0, (255 - WheelPos * 3) * BRIGHTNESS / 255.0 );
  }
}

void paintWord(int arrWord[], uint32_t intColor) {
  for (int i = 0; i < grid.numPixels() + 1; i++) {
    if (arrWord[i] == -1) {
      grid.show();
      break;
    } else {
      grid.setPixelColor(arrWord[i], intColor);
    }
  }
}

void spellWord(int arrWord[], uint32_t intColor) {
  for (int i = 0; i < grid.numPixels() + 1; i++) {
    if (arrWord[i] == -1) {
      break;
    } else {
      grid.setPixelColor(arrWord[i], intColor);
      grid.show();
      delay(500);
    }
  }
}


/*---------------------------------
   These functions are for
   programming the DS3231
   --------------------------------- */

// print out the software version number
void printInstructions(void) {
  Serial.println(F("--------Hello Richard!--------"));
  Serial.println(F("To set the time, enter Y, M, or S followed by a number, followed by a newline character"));
  Serial.println(F("Example: to set the year to 2017:"));
  Serial.println(F("y2017"));
  Serial.println(F("------------------------"));
  Serial.println(F("------------------------"));
  Serial.println(F("The current time is:"));
  digitalClockDisplay();
  Serial.println(F("------------------------"));
  Serial.println(F("The brightness is set to:"));
  Serial.println(BRIGHTNESS);
  Serial.println(F("------------------------"));
}
