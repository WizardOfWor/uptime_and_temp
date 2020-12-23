/*
 uptime_and_temp
 by Daniel Britton
 
 This sketch reads uptime in seconds and temperature data from a
 connected USB serial port and displays it on the LCD.  In between
 periodic updates of the PC's uptime, it updates the time every second.

 Based on the LiquidCrystal Library demo that ships with Arduino IDE,
 original documentation and attributions below.
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/
#include <Timezone.h>           // http://github.com/JChristensen/Timezone
#include <time.h>

// include the library code:
#include <LiquidCrystal.h>

#ifndef   UNIX_OFFSET
#define   UNIX_OFFSET   946684800
#endif

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int doSerial = true;
unsigned long lastMs = 0L;

#define SHOW_TIME 0
#define SHOW_UPTIME 1
#define TIME_DISPLAY_INTERVAL 5
int toShow = SHOW_UPTIME;

time_t eastern, utc;
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  //UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   //UTC - 5 hours
Timezone usEastern(usEDT, usEST);


void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.noDisplay();

  if (doSerial)
    Serial.begin(9600);

  lastMs = millis() / 1000L;
}

#define XFER_BUFSIZ 20
byte buf[XFER_BUFSIZ];
int bp = 0;
long upTime = 0, dispTime = 0, loTemp = 32, curTemp = 50, hiTemp = 99;
long displayTime = 0;
time_t epochSecs;

void loop() {
  if (doSerial) {
    if (Serial.available()) {
      while (Serial.available() && bp < XFER_BUFSIZ) {
        buf[bp++] = Serial.read();
      }
    }
  
    if (bp == XFER_BUFSIZ) {
      lcd.display();
      upTime    = *(long*)&buf[0];
      dispTime  = *(long*)&buf[4];
      loTemp    = *(long*)&buf[8];
      curTemp   = *(long*)&buf[12];
      hiTemp    = *(long*)&buf[16];
      bp = 0;
      
      // set the cursor to column 0, line 0
      lcd.setCursor(0, 0);
      lcd.print(curTemp);
      lcd.print(char(223));
      lcd.print(" L ");
    
      lcd.print(loTemp);
      lcd.print(char(223));
      
      lcd.print(" H ");
    
      lcd.print(hiTemp);
      lcd.print(char(223));
      lcd.print("  ");
      lastMs = millis() / 1000L;
    }
  }

  unsigned long curMs = millis() / 1000L;
  if (curMs > lastMs) {
    long delta = curMs - lastMs;
    upTime += delta;
    lastMs = curMs;

    displayTime += delta;
    dispTime += delta;
    if (displayTime > TIME_DISPLAY_INTERVAL) {
      toShow = toShow == SHOW_TIME ? SHOW_UPTIME : SHOW_TIME;
      displayTime = 0;
      // set the cursor to column 0, line 1
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }
  }
  
  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);

  if (toShow == SHOW_TIME) {
    TimeChangeRule *tcr;        // pointer to the time change rule, use to get the TZ abbrev
    char m[4];    // temporary storage for month string (DateStrings.cpp uses shared buffer)
    char buf[40];

    epochSecs = time_t(dispTime - UNIX_OFFSET);
    time_t t = usEastern.toLocal(epochSecs, &tcr);    
    strcpy(m, monthShortStr(month(t)));
    sprintf(buf, "%2.2d:%.2d:%.2d %2.2d/%2.2d", hour(t), minute(t), second(t), month(t), day(t));
            
    lcd.print(buf);
  } else if (toShow == SHOW_UPTIME) {
    // print the number of seconds since reset:
    unsigned long d = upTime / (24L * 60L * 60L);
    lcd.print(d);
    lcd.print("d ");
    unsigned long h = (upTime / (60L * 60L)) % 24L;
    lcd.print(h);
    lcd.print("h ");
    unsigned long m = (upTime / 60L) % 60L;
    lcd.print(m);
    lcd.print("m ");
    unsigned long s = upTime % 60L;
    lcd.print(s);
    lcd.print("s ");
    lcd.print("   ");
  }
}
