/*
  LiquidCrystal Library - uptime_and_temp

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

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

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 9, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int doSerial = true;
unsigned long lastMs = 0L;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.noDisplay();

  if (doSerial)
    Serial.begin(9600);

  lastMs = millis() / 1000L;
}

byte buf[16];
int bp = 0;
long upTime = 0, loTemp = 32, curTemp = 50, hiTemp = 99;

void loop() {
  if (doSerial) {
    if (Serial.available()) {
      while (Serial.available() && bp < 16) {
        buf[bp++] = Serial.read();
      }
    }
  
    if (bp == 16) {
      lcd.display();
      upTime  = *(long*)&buf[0];
      loTemp  = *(long*)&buf[4];
      curTemp = *(long*)&buf[8];
      hiTemp  = *(long*)&buf[12];
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
    upTime += curMs - lastMs;
    lastMs = curMs;
  }
  
  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);
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
