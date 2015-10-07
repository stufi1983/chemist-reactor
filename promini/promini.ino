#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define DEBUGLCD

SoftwareSerial mySerial(2, 3); // RX, TX
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);

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

  mySerial.begin(1200);

  pinMode(ledPin, OUTPUT);

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(F("Teknik Elektro!"));
}
byte buff = 0;
void loop() { // run over and over

  //forward softSerial to Serial and debug to LCD if DEBUGLCD is defined
  if (mySerial.available()) {
    buff = mySerial.read();
    Serial.write(buff);   
#ifdef  DEBUGLCD    
    if (buff == 13) {
      if (lcdCur == 0) {
        lcdCur = 1;
        lcd.setCursor(0, lcdCur);
      } else {
        lcdCur = 0;
        lcd.setCursor(0, lcdCur);
      }
    }
    lcd.write(buff);
#endif
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
