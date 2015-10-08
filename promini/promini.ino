#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define DEBUGLCD

SoftwareSerial mySerial(2, 3); // RX, TX
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
#define COL  16
#define ROW  2
byte bytes[16];
byte byteNum = 0;

const int ledPin =  13;      // the number of the LED pin

int lcdCur = 0;
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
long interval = 1000;           // interval at which to blink (milliseconds)

void setup() {
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("Teknik Elektro!"));

  mySerial.begin(300);

  pinMode(ledPin, OUTPUT);

  lcd.begin(COL, ROW);
  lcd.setCursor(0, 0);
  lcd.print(F("Teknik Elektro!"));
}
byte buff = 0;
void loop() { // run over and over

  //forward softSerial to Serial and debug to LCD if DEBUGLCD is defined
  if (mySerial.available()) {

    buff = mySerial.read();
    if (byteNum < COL) {
      bytes[byteNum] = buff;
      byteNum++;
    }

#ifdef  DEBUGLCD
    if (buff == 12) {

      if (lcdCur == 0) {
        lcdCur = 1;
      } else {
        lcdCur = 0;
      }

      lcd.setCursor(0, lcdCur);
      for (byte i = 0; i < COL; i++) {
        lcd.write(bytes[i]);
        bytes[i] = 0x20;
      }

      byteNum = 0;
    }
#endif
  }
  /*
    if (byteNum > 0) {
      for (byte i = 0; i < 16; i++) {
        Serial.write(buff);
      }
      byteNum = 0;
    }
  */
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
