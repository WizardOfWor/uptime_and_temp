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

#include <math.h>

#include <Timezone.h>           // http://github.com/JChristensen/Timezone
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
char dispBuf[32];
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

      // Examples to Fit Text in Display
      // 1234567890123456        1234567890123456
      // 99d L 99d H 99d                              Most of the world
      // 100d L 100d H 100d  --> 110d L100d H100d     Someplace hot
      // -10d L -10d H -10d  --> -10d L-10d H-10d     Someplace not
      int pad = 0, padlimit = 0;
      sprintf(dispBuf, "%ld%c L %ld%c H %ld%c", curTemp, char(223), loTemp, char(223), hiTemp, char(223));
      if (strlen(dispBuf) >= 16) {
        // If the temp display won't fit in the 16-digit display, compact it by removing spaces.
        sprintf(dispBuf, "%ld%c L%ld%c H%ld%c", curTemp, char(223), loTemp, char(223), hiTemp, char(223));        
      } else {
        // Center the text as long as the display string is less than 16 characters.
        pad = (16 - strlen(dispBuf)) / 2;
        // If exactly 15 characters, indent by one.
        padlimit = max(pad, 1);
      }
      for (int i = 0; i < padlimit; i++)
        lcd.print(" ");
      lcd.print(dispBuf);
      for (int i = 0; i < pad; i++)
        lcd.print(" ");      
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
    epochSecs = time_t(dispTime);
    time_t t = usEastern.toLocal(epochSecs, &tcr);
    setTime(t);    
    sprintf(dispBuf, "%d:%.2d:%.2d%c %2.2d/%2.2d", hourFormat12(), minute(), second(), isAM() ? 'A' : 'P', month(), day());

    // 1234567890123456
    // 1:00:00P 1/25
    // 12:00:00P 12/25
    int pad = (16 - strlen(dispBuf)) / 2;
    for (int i = 0; i < max(pad, 1); i++)
      lcd.print(" ");
    lcd.print(dispBuf);
    for (int i = 0; i < pad; i++)
      lcd.print(" ");
  } else if (toShow == SHOW_UPTIME) {
    // 1234567890123456
    // 1d 0h 0m 0s
    // 999d 23h 59m 59s
    unsigned long d = upTime / (24L * 60L * 60L);
    unsigned long h = (upTime / (60L * 60L)) % 24L;
    unsigned long m = (upTime / 60L) % 60L;
    unsigned long s = upTime % 60L;
    sprintf(dispBuf, "%ldd %ldh %ldm %lds", d, h, m, s);
    int pad = (16 - strlen(dispBuf)) / 2;
    for (int i = 0; i < pad; i++)
      lcd.print(" ");
    lcd.print(dispBuf);
    for (int i = 0; i < pad; i++)
      lcd.print(" ");
  }
}
