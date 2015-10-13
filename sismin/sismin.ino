//ATMega8535 8 MHz
//
#define FLOWINTERRUPT   //koneksi flow sensor,pilih: FLOWINTERRUPT di pin INT0/16 atau FLOWDIGITAL di pin PA1/39

//NB: urutan pin header untukflow sensor di PCB tidak pas dengan sensor
#define NOTCEKFLOW         //test Flow Sensor, pilih: CEKFLOW untuk test atau NOTCEKFLOW untuk normal
#define NOTCEKPH           //test ph meter, pilih: CEKPH untuk test atau NOTCEKPH untuk normal
#define NOTCEKKP           //test keypad
#define CEKPOT          //test potensio pengatur putaran
#define PL(x) {Serial.println(x);  Serial.write(12);}
#define P(x) {Serial.print(x);}
#define L() Serial.write(12);}
#define W(x) Serial.write(x);}

#define BUZZER   14

#define  KATUP1  4//pb4
#define  KATUP2  3//pb3
#define  KATUP3  1//pb1
#define  KATUP4  0//pb0

//Timer masih dummy
#define TIMERBAKA  30
#define TIMERBAKB  30
#define TIMERBAKC  30
#define TIMERBAKD  12  //1800 
#define TIMERBAKE  12  //3600*4 
#define TIMERBAKF  12  //3600 

#define MOTORA    23 //PC7  --->HARUSNYA DI OUTPUT TIMER 
int BakARpm = 50;
#define MOTORB    22 //PC6
int BakBRpm = 50;

#define MOTORC    12 //PD4  --->HARUSNYA DI OUTPUT TIMER
#define POTMOTORC A7
int BakCRpm = 100;

#define MOTORD    13 //PD5  --->HARUSNYA DI OUTPUT TIMER
#define POTMOTORD A6
int BakDRpm = 50;

#define PERISA    19 //PC3
#define PERISB    18 //PC2

#define MAXDIGIT  6

#define DEBITPERISB 10 //10 mL/detik
#define DEBITPERISA 10 //10 mL/detik

int debitPB = 0;
int debitPA = 0;

#define COL   16
byte bytes[COL];
byte byteNum = 0;
byte int2bytebuf[MAXDIGIT];

int timerBakC = 0;
int timerBakD = 0; //30 minutes
int timerBakE = 0; //1 hour
int timerBakF = 0; //1 hour

#define FLOWSENSOR  A1 //PA1 -->HARUSNYA DI PIN INTERRUPT
int debit = 1;
int RisingEdgeCounter = 0;
volatile int RisingEdgeCounterInt;
unsigned long startMillis;
unsigned long duration = 1000; //1000 ms
unsigned long currMillis = 0;
bool buttonState = LOW;
bool lastButtonState = LOW;

int kadarKoa = 0;
unsigned long volKoagulan = 0;
int dayaIkat = 0;

#define PHMETER  A0
int ph = 72;
#define Offset -0.38        //kalibrasi jika perlu
#define samplingInterval 2
#define phReadDuration 1000
#define ArrayLenth  40    //jumlah sample per frame
int pHArray[ArrayLenth];
int pHArrayIndex = 0;
static float pHValue, lastpHValue, voltage;
static unsigned long samplingTime;
bool ok = false;

/*
Note: Sertiap mensampling, pH harus dicuci/dibersihkan dari sisa larutan yang menempel di elektroda dengan larutan standar keasaman 7.0  ---> kalau di bak harus air mengalir ????
Cara kalibrasi:
1. Taruh elektroda pada larutan pH 7.0 (atau hubungkan singkat konektor BNC).
2. Bandingkan pH dengan yang terbaca, jika tidak 7.0 maka sesuaikan offset.
   Contoh: pH terbaca 6.88, maka offset (selisih) 0.12 dituliskan di "# define Offset 0.12".
3. Taruh elektroda pH pada larutan pH 4.00 dan tunggu hingga stabil
4. Kalibrasi potension hingga pH terbaca 4.00.
*/


int volLarDisinfect = 1;


bool keypadEntry = false;
int inputVal = 0;

byte state = 0;
unsigned long startMilis;

bool toogle = LOW;

void setup() {
  Serial.begin(300);
  resetPin();
#ifdef CEKFLOW
  state = 2;
#endif

#ifdef CEKPH
  state = 8;
#endif

#ifdef CEKKP
  state = 8;
#endif
}

void resetPin() {

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  pinMode(KATUP1, OUTPUT);
  digitalWrite(KATUP1, LOW);
  pinMode(KATUP2, OUTPUT);
  digitalWrite(KATUP2, LOW);
  pinMode(KATUP3, OUTPUT);
  digitalWrite(KATUP3, LOW);
  pinMode(KATUP4, OUTPUT);
  digitalWrite(KATUP4, LOW);

  pinMode(MOTORA, OUTPUT);
  digitalWrite(MOTORA, LOW);
  pinMode(MOTORB, OUTPUT);
  digitalWrite(MOTORB, LOW);
  pinMode(MOTORC, OUTPUT);
  digitalWrite(MOTORC, LOW);
  pinMode(MOTORD, OUTPUT);
  digitalWrite(MOTORD, LOW);

  pinMode(PERISA, OUTPUT);
  digitalWrite(PERISA, LOW);
  pinMode(PERISB, OUTPUT);
  digitalWrite(PERISB, LOW);

  pinMode(FLOWSENSOR, INPUT);
  digitalWrite(FLOWSENSOR, HIGH);

}

void loop() {
  int potCValue = 0;
  int potDValue = 0;

  switch (state) {
    case 0:
      for (byte i = 0; i < 5; i++) {
        //1 Tampil judul
        PL("D1");
        delay(1000);
      }
      state++;
      break;

    case 1:
      while (!keypadEntry) {
        //2 Bak C DC 100%
        potCValue = analogRead(POTMOTORC);
        analogWrite(MOTORC, map(potCValue, 0, 1023, 0, 255));
        BakCRpm = potCValue / 10; if (BakCRpm > 100) BakCRpm = 100;

        //3 Bak D 50%
        potDValue = analogRead(POTMOTORD);
        analogWrite(MOTORC, map(potDValue, 0, 1023, 0, 255));
        BakDRpm = potDValue / 10; if (BakDRpm > 100) BakDRpm = 100;

        //4 Tampil kondisi Motor Pengaduk
        beep(100);
        P("D2");
        int2byte(BakCRpm, 3);
        int2byte(BakDRpm, 3);
        Serial.write(12);
        delay(500); //eliminate LCD blinking
      }
#ifdef NOTCEKPOT
        beep(100);
        state++;
#endif
      break;

    case 2:
      //5 hitung debit dr sensor
      //Harusnya sensor ada di pin Interrupt, krn prinsip kerjanya menghitung rising edge dalam 1 detik
      //Jadi mungkin tidak presisi
      RisingEdgeCounter = 0;
      startMillis = millis();
      debit = 1;

#ifdef FLOWINTERRUPT //pakai interrupt 0 / PD2 / D10 /pin ic 16
      RisingEdgeCounterInt = 0;
      attachInterrupt(0, flowInterrupt, RISING); // Setup Interrupt
      //sei();
      while (millis() - startMillis < duration) {
        //counting....
      }
      //cli();
      debit = (RisingEdgeCounterInt * 60 / 7.5); //Flow rate per jam = (Pulse frequency x 60) / 7.5Q
      RisingEdgeCounterInt = 0;
#else      //pakai digital pin kadang tidak pas    
      while (millis() - startMillis < duration) {
        digitalWrite(FLOWSENSOR, HIGH);
        buttonState = digitalRead(FLOWSENSOR);
        if (buttonState != lastButtonState) { //toggle
          if (buttonState == HIGH) { //rissing edge
            RisingEdgeCounter++;
            lastButtonState = buttonState;
          }
        }
      }
      debit = (RisingEdgeCounter * 60 / 7.5); //Flow rate per jam = (Pulse frequency x 60) / 7.5Q
#endif

      debit = debit * 1000;  //ml per Jam
      debit = debit / 3600; //ml per detik


#ifdef CEKFLOW     //hack untuk cek flowmeter
      beep(100);
      P("D4");
      int2byte(timerBakC, 2);
      int2byte(debit, 4);
      Serial.write(12);
      delay(100);
#else
      //6
      beep(100);
      P("D3");
      Serial.write(12);
      delay(2000);

      kadarKoa = 4999;                                   //dummy, baca keypad  ---------

      beep(100);
      P("D3"); int2byte(kadarKoa, 4);
      Serial.write(12);
      delay(2000);
      state++;
#endif
      break;

    case 3:
      //7
      volKoagulan = (kadarKoa * debit * 1000) / 50;
      //8 TAMPILKAN NILAI VOLUME TERHITUNG
      beep(100);
      P("D4");
      int2byte(timerBakC, 2);
      int2byte(debit, 3);
      Serial.write(12);
      //9 aktifkan peristaltik B n detik
      //dummy, delay berdasar volKoagulan / debitPB setiap detik, sementara 1000 ms  ---------
      debitPB = DEBITPERISB;
      digitalWrite(PERISB, HIGH); delay(volKoagulan / debitPB * 1000); digitalWrite(PERISB, LOW);
      //10 set timer 30 detik
      timerBakC = 0;
      beep(100);
      for (byte i = 0; i < 5; i++) {
        P("D4");
        int2byte(timerBakC, 2);
        int2byte(debit, 3);
        Serial.write(12);
        delay(1000); timerBakC++;
      }
      state++;
      beep(100);
      break;

    case 4:
      //14 while timerBakC >0

      P("D5");
      int2byte(timerBakC, 2);
      int2byte(volKoagulan, 6);
      Serial.write(12);
      delay(1000); timerBakC++;
      if (timerBakC >= TIMERBAKC) {
        beep(100);
        state++;
      }
      break;

    case 5:
      //12 timer off
      P("D6");
      int2byte(timerBakC, 2);
      Serial.write(12);
      timerBakD = 0;
      delay(2000);
      //aktifkan katup 1
      digitalWrite(KATUP1, HIGH);
      state++;       beep(100);
      break;

    case 6:
      //14 //while timerBakD >0
      P("D7");
      int2byte((timerBakD) / 60, 2); //minutes
      int2byte((timerBakD) % 60, 2); //seconds
      Serial.write(12);
      delay(1000); timerBakD++;
      if (timerBakD >= TIMERBAKD) {
        state++;
        beep(100);
      }
      break;

    case 7:
      //15 matikan timer
      //aktifkan katup 2
      digitalWrite(KATUP1, HIGH);

      P("D8");
      int2byte((timerBakD) / 60, 2);
      Serial.write(12);
      delay(2000);//dummy

      //16 tampilkan daya ikat klor
      beep(100);
      P("D9");
      Serial.write(12);
      delay(2000);

      dayaIkat = 2;                                             //dummy, baca keypad  ---------

      P("D9"); int2byte(kadarKoa, 4);
      Serial.write(12);
      beep(100);
      delay(2000);
#ifdef NOTCEKKP
      state++;
#endif
      break;

    case 8:
      //17 baca sensor ph interval 1 detik
#ifdef NOTCEKPH
      P("D0");
      Serial.write(12);
#endif
      pHValue = 0; lastpHValue = -1;
      while ((int)(pHValue * 10) != (int)(lastpHValue * 10)) { //jika ph belum stabil per 1/10  (phlalu !=ph sekarang)
        lastpHValue = pHValue;
        startMillis = millis();
        while (millis() - startMillis < phReadDuration) {
          samplingTime = millis();
          if (millis() - samplingTime > samplingInterval)
          {
            toogle = !toogle;
            digitalWrite(13, toogle);
            pHArray[pHArrayIndex++] = analogRead(PHMETER);
            if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
            voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
            pHValue = 3.5 * voltage + Offset;
            samplingTime = millis();
          }
        }
      }

      ph = (unsigned long)(pHValue * 10);

      //hitng volLarDisinfect berdasar tabel gunakan variable pHValue, ph hanya untuk tampilan LCD
      volLarDisinfect = hitungVolLarDisinfektan(pHValue);

      beep(100);
      P("D:");
      int2byte((int)ph / 10 , 1);
      int2byte((int)ph % 10, 1);
      int2byte(volLarDisinfect, 3);
      Serial.write(12);
      beep(100);

#ifdef NOTCEKPH
      delay(5000);
      //18 aktifkan peristaltik A n detik
      debitPA = DEBITPERISA;
      digitalWrite(PERISA, HIGH); delay(volLarDisinfect / debitPA * 1000); digitalWrite(PERISA, LOW);
      //19 start timer 4 (4 jam)
      timerBakE = 0;
      state++;
#endif
      break;

    case 9:
      //20 while timerBakE > 0
      P("D;");
      int2byte((timerBakE) / 60, 3); //minutes
      int2byte((timerBakE) % 60, 2); //seconds
      Serial.write(12);
      //}while timerBakE > 0
      delay(1000);
      timerBakE++;
      if (timerBakE >= TIMERBAKE) {
        beep(100);
        state++;
      }
      break;

    case 10:
      //21 matikan timer
      beep(100);
      P("D<");
      int2byte((timerBakE) / 3600, 1);
      Serial.write(12);
      delay(2000);
      //22 aktifkan timer 1 jam
      timerBakF = 0;
      state++;
      break;

    case 11:
      //23 while timerBakF 1 jam > 0 {
      P("D=");
      int2byte((timerBakF) / 60, 2);
      int2byte((timerBakF) % 60, 2);
      Serial.write(12);
      delay(1000);
      timerBakF++;
      if (timerBakF >= TIMERBAKF) {
        state++;
        beep(100);
      }
      break;

    case 12:
      //24 matikan timer aktifkan katup 4
      P("D>");
      int2byte((timerBakF) / 3600, 1);
      Serial.write(12);
      delay(2000);
      //selesai ulang dari awal
      state++;
      break;

    case 13:
      //dummy, baca keypad  untuk repeat / stop (default stop)---------
      resetPin();
      break;

    default:
      break;
  }
}

void int2byte(int val, byte digit) {
  int2byte((unsigned long)val, digit);
}
void int2byte(unsigned long val, byte digit) {
  //satuan
  int2bytebuf[5] = (val % 10) + 0x30;
  if (val <= 9) {
    int2bytebuf[4] = 0x20;
  } else {
    int2bytebuf[4] = ( ( (val / 10) % 10) + 0x30);
  }
  if (val <= 99) {
    int2bytebuf[3] = 0x20;
  } else {
    int2bytebuf[3] = ( ( (val / 100) % 10) + 0x30);
  }
  if (val <= 999) {
    int2bytebuf[2] = 0x20;
  } else {
    int2bytebuf[2] = ( ( (val / 1000) % 10) + 0x30);
  }
  if (val <= 9999) {
    int2bytebuf[1] = 0x20;
  } else {
    int2bytebuf[1] = ( ( (val / 10000) % 10) + 0x30);
  }
  if (val <= 99999) {
    int2bytebuf[0] = 0x20;
  } else {
    int2bytebuf[0] = ( ( (val / 100000) % 10) + 0x30);
  }
  for (byte i = MAXDIGIT - digit; i < MAXDIGIT; i++) {
    Serial.write(int2bytebuf[i]);
  }
}

double avergearray(int* arr, int number) {
  int i;
  int max, min;
  double avg;
  unsigned long amount = 0;
  if (number <= 0) {
    PL("D1");
    delay(1000);
    return 0;
  }
  if (number < 5) { //less than 5, calculated directly statistics
    for (i = 0; i < number; i++) {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  } else {
    if (arr[0] < arr[1]) {
      min = arr[0]; max = arr[1];
    }
    else {
      min = arr[1]; max = arr[0];
    }
    for (i = 2; i < number; i++) {
      if (arr[i] < min) {
        amount += min;      //arr<min
        min = arr[i];
      } else {
        if (arr[i] > max) {
          amount += max;  //arr>max
          max = arr[i];
        } else {
          amount += arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount / (number - 2);
  }//if
  return avg;
}



void flowInterrupt ()                  // Interruot function
{
  RisingEdgeCounterInt++;
}

void beep(byte dur) {
  digitalWrite(BUZZER, HIGH);
  delay(dur);
  digitalWrite(BUZZER, LOW);
}

int hitungVolLarDisinfektan(int pHValue) {
  if ( pHValue >= 6 && pHValue < 7) return 1;
  if ( pHValue >= 7 && pHValue < 8) return 2;

}
