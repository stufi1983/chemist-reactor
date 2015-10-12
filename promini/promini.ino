#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

//#define DEBUGLCD
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
byte displayNum = 0;

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

  //LCD buffer
  for (byte i = 0; i < COL; i++) {
    bytes[i] = 0x20;
  }

  lcd.setCursor(0, 0); lcd.print(F("Connecting ....."));

  //Display(1);
  //delay(5000);

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

    if (bytes[0] == 0x44) {
      Display(bytes[1] - 0x30);
    }

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

void Display(byte tampil )
{
  displayNum = tampil - 0x30; lcd.noCursor();
  switch (tampil) {
    case  0:
      lcd.setCursor(0, 0); lcd.print(F(" MEMBACA SENSOR "));
      lcd.setCursor(0, 1); lcd.print(F(" HARAP MENUNGGU "));
      break;
    case  1:
      lcd.setCursor(0, 0); lcd.print(F("ALAT KOAGULASI &"));
      lcd.setCursor(0, 1); lcd.print(F("DISINFEKTAN AIR "));
      break;
    case  2:
      lcd.setCursor(0, 0); lcd.print(F(" BAK C=    RPM  "));
      lcd.setCursor(0, 1); lcd.print(F(" BAK D=    RPM  "));
      lcd.setCursor(8, 0);
      for (byte x = 0; x < 3; x++) {
        lcd.write(bytes[x + 2]);
      }
      lcd.setCursor(8, 1);
      for (byte x = 0; x < 3; x++) {
        lcd.write(bytes[x + 5]);
      }
      break;
    case  3:
      lcd.setCursor(0, 0); lcd.print(F(" KADAR KOAGULAN "));
      lcd.setCursor(0, 1); lcd.print(F("ENTRY=      mg/s"));
      lcd.setCursor(7, 1);
      
      if (bytes[2] == 12) {
         lcd.cursor();
      } else {
        for (byte x = 0; x < 4; x++) {
          lcd.write(bytes[x + 2]);
        }
      }

      break;
    case  4:
      lcd.setCursor(0, 0); lcd.print(F("TIMER BAK-C=   s"));
      lcd.setCursor(0, 1); lcd.print(F("DEBIT=      mL/s"));
      lcd.setCursor(13, 0);
      for (byte x = 0; x < 2; x++) {
        lcd.write(bytes[x + 2]);
      }
      lcd.setCursor(7, 1);
      for (byte x = 0; x < 4; x++) {
        lcd.write(bytes[x + 4]);
      }
      break;
    case  5:
      lcd.setCursor(0, 0); lcd.print(F("TIMER BAK-C=   s"));
      lcd.setCursor(0, 1); lcd.print(F("KOAG=       mg/L"));
      lcd.setCursor(13, 0);
      for (byte x = 0; x < 2; x++) {
        lcd.write(bytes[x + 2]);
      }
      lcd.setCursor(6, 1);
      for (byte x = 0; x < 6; x++) {
        lcd.write(bytes[x + 4]);
      }
      break;
    case  6:
      lcd.setCursor(0, 0); lcd.print(F("TIMER BAK-C=   s"));
      lcd.setCursor(0, 1); lcd.print(F("KATUP 1 AKTIF   "));
      lcd.setCursor(13, 0);
      for (byte x = 0; x < 2; x++) {
        lcd.write(bytes[x + 2]);
      }
      break;
    case  7:
      lcd.setCursor(0, 0); lcd.print(F("  TIMER BAK-D=  "));
      lcd.setCursor(0, 1); lcd.print(F("  MENIT   DETIK "));
      lcd.setCursor(0, 1);
      for (byte x = 0; x < 2; x++) {
        lcd.write(bytes[x + 2]);
      }
      lcd.setCursor(8, 1);
      for (byte x = 0; x < 2; x++) {
        lcd.write(bytes[x + 4]);
      }
      break;
    case  8:
      lcd.setCursor(0, 0); lcd.print(F("TIMER BAK-D=   M"));
      lcd.setCursor(0, 1); lcd.print(F("KATUP 2 AKTIF   "));
      lcd.setCursor(13, 0);
      for (byte x = 0; x < 2; x++) {
        lcd.write(bytes[x + 2]);
      }
      break;
    case  9:
      lcd.setCursor(0, 0); lcd.print(F("DAYA IKAT CHLOR "));
      lcd.setCursor(0, 1); lcd.print(F("ENTRY=      mg/s"));
      lcd.setCursor(7, 1);
      
      if (bytes[2] == 12) {
         lcd.cursor();
      } else {
        for (byte x = 0; x < 4; x++) {
          lcd.write(bytes[x + 2]);
        }
      }

      break;
    case  10:
      lcd.setCursor(0, 0); lcd.print(F("LEVEL pH  = ,   "));
      lcd.setCursor(0, 1); lcd.print(F("VOL DISINF=   mL"));
      lcd.setCursor(9, 0);
      for (byte x = 0; x < 1; x++) {
        lcd.write(bytes[x + 2]);
      }
      lcd.setCursor(11, 0);
      for (byte x = 0; x < 1; x++) {
        lcd.write(bytes[x + 3]);
      }
      lcd.setCursor(11, 1);
      for (byte x = 0; x < 3; x++) {
        lcd.write(bytes[x + 4]);
      }
      break;
    case  11:
      lcd.setCursor(0, 0); lcd.print(F("  TIMER BAK-E=  "));
      lcd.setCursor(0, 1); lcd.print(F("   MENIT   DETIK"));
      lcd.setCursor(0, 1);
      for (byte x = 0; x < 3; x++) {
        lcd.write(bytes[x + 2]);
      }
      lcd.setCursor(8, 1);
      for (byte x = 0; x < 2; x++) {
        lcd.write(bytes[x + 5]);
      }
      break;
    case  12:
      lcd.setCursor(0, 0); lcd.print(F("TIMER BAK-E= JAM"));
      lcd.setCursor(0, 1); lcd.print(F("KATUP 3 AKTIF   "));
      lcd.setCursor(12, 0);
      for (byte x = 0; x < 1; x++) {
        lcd.write(bytes[x + 2]);
      }
      break;
    case  13:
      lcd.setCursor(0, 0); lcd.print(F("  TIMER BAK-F=  "));
      lcd.setCursor(0, 1); lcd.print(F("  MENIT   DETIK "));
      lcd.setCursor(0, 1);
      for (byte x = 0; x < 2; x++) {
        lcd.write(bytes[x + 2]);
      }
      lcd.setCursor(8, 1);
      for (byte x = 0; x < 2; x++) {
        lcd.write(bytes[x + 4]);
      }
      break;
      break;
    case  14:
      lcd.setCursor(0, 0); lcd.print(F("TIMER BAK-F= JAM"));
      lcd.setCursor(0, 1); lcd.print(F("KATUP 4 AKTIF   "));
      lcd.setCursor(12, 0);
      for (byte x = 0; x < 1; x++) {
        lcd.write(bytes[x + 2]);
      }
      break;
    default:
      break;
  }
}




void DisplayValue(byte col, byte row, byte digit, byte start) {
  lcd.setCursor(col, row);
  for (byte x = 0; x < digit; x++) {
    lcd.write(bytes[x + start]);
  }
}
