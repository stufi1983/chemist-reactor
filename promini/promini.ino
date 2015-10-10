#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define DEBUGLCD
#define COL  16
#define ROW  2

SoftwareSerial mySerial(2, 3); // RX, TX
//LCD: 1 GND; 2 VCC; 3 CONTRAST; 5 GND; 7,8,9,10 NC
LiquidCrystal lcd(9, 8, 7, 6, 5, 4); //4,6,11,12,13,14

byte bytes[16];
byte byteNum = 0;

const int ledPin =  13;      // the number of the LED pin
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
long interval = 1000;           // interval at which to blink (milliseconds)

bool show = false;
int lcdCur = 0;
byte buff = 0;

void setup() {
  
  //ProMini to PC
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("Loading...!"));
  
  //ProMini to SisMin
  mySerial.begin(300);

  //Status Led 
  pinMode(ledPin, OUTPUT);

  //LCD initialisation
  lcd.begin(COL, ROW);
  lcd.setCursor(0, 0);
  lcd.print(F("Loading...!"));

  //LCD buffer 
  for (byte i = 0; i < COL; i++) {
    bytes[i] = 0x20;
  }

}


void loop() {

  //forward softSerial to Serial and debug to LCD if DEBUGLCD is defined
  if (mySerial.available()) {

    buff = mySerial.read();
    if (buff != 0x0D && buff != 0x0A) {
      if (byteNum < COL) {
        bytes[byteNum] = buff;
        byteNum++;
      }
    }

    if (buff == 12) {
      show = true;
    }
  }

  if (show) {
    show = false;

#ifdef  DEBUGLCD
    if (lcdCur == 0) {
      lcdCur = 1;
    } else {
      lcdCur = 0;
    }

    lcd.setCursor(0, lcdCur);
    for (byte i = 0; i < COL; i++) {

      if (i < byteNum - 1) {
        lcd.write(bytes[i]);
      }
      bytes[i] = 0x00;
    }

#endif
    byteNum = 0;

  }

  //forward Serial to softSerial
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }

  //blinky
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;
      interval = 100;
    }
    else {
      ledState = LOW;
      interval = 900;
    }
    digitalWrite(ledPin, ledState);
  }

}
