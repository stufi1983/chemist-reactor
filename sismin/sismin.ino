//ATMega8535 8 MHz
//
#define PL(x) {Serial.println(x);  Serial.write(12);}
#define P(x) {Serial.print(x);}
#define L() Serial.write(12);}
#define W(x) Serial.write(x);}

#define TIMERBAKA  30
#define TIMERBAKB  30
#define TIMERBAKC  10  //30 
#define TIMERBAKD  12  //1800 
#define TIMERBAKE  12  //3600*4 
#define TIMERBAKF  12  //3600 

#define MOTORA    23 //PC7  --->HARUSNYA DI OUTPUT TIMER 
int BakARpm = 50;
#define MOTORB    22 //PC6
int BakBRpm = 50;
#define MOTORC    21 //PC5  --->HARUSNYA DI OUTPUT TIMER
int BakCRpm = 100;
#define MOTORD    20 //PC4  --->HARUSNYA DI OUTPUT TIMER
int BakDRpm = 50;

#define PERISA    19 //PC3
#define PERISB    18 //PC2

#define MAXDIGIT  6

#define COL   16
byte bytes[COL];
byte byteNum = 0;
byte int2bytebuf[MAXDIGIT];

int timerBakC = 0;
int timerBakD = 0; //30 minutes
int timerBakE = 0; //1 hour
int timerBakF = 0; //1 hour

#define FLOWSENSOR  30 //PA1 -->HARUSNYA DI PIN INTERRUPT
int debit = 1;
int RisingEdgeCounter = 0;
unsigned long startMillis;
unsigned long duration = 1000; //1000 ms
unsigned long currMillis = 0;
bool buttonState = LOW;
bool lastButtonState = LOW;

int kadarKoa = 0;
long volKoagulan = 0;
int dayaIkat = 0;

#define PHMETER  A0
int ph = 72;
#define Offset 0.00        //kalibrasi jika perlu
#define samplingInterval 20
#define phReadDuration 2000
#define ArrayLenth  40    //jumlah sample per frame
int pHArray[ArrayLenth];   
int pHArrayIndex = 0;
static float pHValue, voltage;
static unsigned long samplingTime;


int volLarDisinfect = 1;


bool keypadEntry = false;
int inputVal = 0;

byte state = 0;
unsigned long startMilis;
void setup() {
  Serial.begin(300);
  resetPin();
}

void resetPin() {

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
  digitalWrite(FLOWSENSOR, LOW);
}

void loop() {
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
      //2 Bak C DC 100%
      digitalWrite(MOTORC, HIGH);

      //3 Bak A, B, D DC 50%
      //TIDAK BISA PWM KRN OUTPUT BUKAN DI PIN TIMER......, SEMENTARA FULL SPEED DULU
      digitalWrite(MOTORA, HIGH); //analogWrite(MOTORA,128);
      digitalWrite(MOTORB, HIGH); //analogWrite(MOTORB,128);
      digitalWrite(MOTORC, HIGH); //analogWrite(MOTORD,128);

      //4 Tampil kondisi Motor Pengaduk
      P("D2");
      int2byte(BakCRpm, 3);
      int2byte(BakDRpm, 3);
      Serial.write(12);
      delay(5000);
      state++;
      break;

    case 2:
      //5 hitung debit dr sensor
      //Harusnya sensor ada di pin Interrupt, krn prinsip kerjanya menghitung rising edge dalam 1 detik
      //Jadi mungkin tidak presisi
      RisingEdgeCounter = 0;
      startMillis = millis();
      while (millis() - startMillis < duration) {
        buttonState = digitalRead(FLOWSENSOR);
        if (buttonState != lastButtonState) { //toggle
          if (buttonState == HIGH) { //rissing edge
            RisingEdgeCounter++;
            lastButtonState = buttonState;
          }
        }
      }
      debit = (RisingEdgeCounter * 60 / 7.5); //Flow rate per jam = (Pulse frequency x 60) / 7.5Q
      debit = debit * 1000;  //ml per Jam
      debit = debit / 3600; //ml per detik

      //6
      P("D3");
      Serial.write(12);
      delay(2000);

      kadarKoa = 4999;                                   //dummy, baca keypad  ---------

      P("D3"); int2byte(kadarKoa, 4);
      Serial.write(12);
      delay(2000);
      state++;
      break;

    case 3:
      //7
      volKoagulan = (kadarKoa * debit * 1000) / 50;
      //8 TAMPILKAN NILAI VOLUME TERHITUNG
      P("D4");
      int2byte(timerBakC, 2);
      int2byte(debit, 3);
      Serial.write(12);
      //9 aktifkan peristaltik B n detik
      //dummy, delay berdasar volKoagulan / debitPB setiap detik, sementara 1000 ms  ---------
      digitalWrite(PERISB, HIGH); delay(1000); digitalWrite(PERISB, LOW);
      //10 set timer 30 detik
      timerBakC = 0;
      for (byte i = 0; i < 5; i++) {
        P("D4");
        int2byte(timerBakC, 2);
        int2byte(debit, 3);
        Serial.write(12);
        delay(1000); timerBakC++;
      }
      state++;
      break;

    case 4:
      //14 while timerBakC >0
      P("D5");
      int2byte(timerBakC, 2);
      int2byte(volKoagulan, 6);
      Serial.write(12);
      delay(1000); timerBakC++;
      if (timerBakC >= TIMERBAKC) state++;
      break;

    case 5:
      //12 timer off
      P("D6");
      int2byte(timerBakC, 2);
      Serial.write(12);
      timerBakD = 0;
      delay(2000);
      //aktifkan katup 1
      state++;
      break;

    case 6:
      //14 //while timerBakD >0
      P("D7");
      int2byte((timerBakD) / 60, 2); //minutes
      int2byte((timerBakD) % 60, 2); //seconds
      Serial.write(12);
      delay(1000); timerBakD++;
      if (timerBakD >= TIMERBAKD) state++;
      break;

    case 7:
      //15 matikan timer
      //aktifkan katup 2
      P("D8");
      int2byte((timerBakD) / 60, 2);
      Serial.write(12);
      delay(2000);//dummy
      //16 tampilkan daya ikat klor
      P("D9");
      Serial.write(12);
      delay(2000);

      dayaIkat = 2;                                             //dummy, baca keypad  ---------

      P("D9"); int2byte(kadarKoa, 4);
      Serial.write(12);
      delay(2000);

      state++;
      break;

    case 8:
      //17 baca sensor ph interval 1 detik
      startMillis = millis();
      while (millis() - startMillis < phReadDuration) {
        samplingTime = millis();
        if (millis() - samplingTime > samplingInterval)
        {
          pHArray[pHArrayIndex++] = analogRead(PHMETER);
          if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
          voltage = avergearray(pHArray, ArrayLenth) * 5.0 / 1024;
          pHValue = 3.5 * voltage + Offset;
          samplingTime = millis();
        }
      }

      ph = (pHValue * 10);   //pakai nilai phValue, ph untuk tampilan saja

      //hitng volLarDisinfect berdasar tabel                                    
      volLarDisinfect = 12;    //volLarDisinfect=vlookup....     //dummy, belum ada tabel untuk lookup  ---------

      P("D:");
      int2byte((int)pHValue*10, 1);
      int2byte((int)pHValue, 1);
      int2byte(volLarDisinfect, 1);
      Serial.write(12);
      delay(5000);
      //18 aktifkan peristaltik A n detik
      //dummy, delay berdasar volLarDisinfect / debitPA setiap detik, sementara 1000 ms ---------
      digitalWrite(PERISA, HIGH); delay(1000); digitalWrite(PERISA, LOW);
      //19 start timer 4 (4 jam)
      timerBakE = 0;
      state++;
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
        state++;
      }
      break;

    case 10:
      //21 matikan timer
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
      break;

    default:
      break;
  }
}

void int2byte(int val, byte digit) {
  int2byte((long)val, digit);
}
void int2byte(long val, byte digit) {
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
  long amount = 0;
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
