#define PL(x) {Serial.println(x);  Serial.write(12);}
#define P(x) {Serial.print(x);}
#define L() Serial.write(12);}
#define W(x) Serial.write(x);}

#define TIMERBAKA  30
#define TIMERBAKB  30
#define TIMERBAKC  60//30 
#define TIMERBAKD  62//1800 
#define TIMERBAKE  62//3600*4 
#define TIMERBAKF  62//3600 

#define MAXDIGIT  6
//ATMega8535 8 MHz

int BakARpm = 50;
int BakBRpm = 50;
int BakCRpm = 100;
int BakDRpm = 50;

int timerBakC = 0;
int timerBakD = 0; //30 minutes
int timerBakE = 0; //1 hour
int timerBakF = 0; //1 hour

int kadarKoa = 0;
int debit = 1;
int volKoagulan = 0;
int dayaIkat = 0;
int ph = 72;
int volLarDisinfect = 1;

byte int2bytebuf[MAXDIGIT];

byte state = 0;
unsigned long startMilis;
void setup() {
  Serial.begin(300);
  digitalWrite(3, HIGH);
}

void loop() {
  switch (state) {
    case 0:
      //1 Tampil judul
      PL("D1");
      delay(5000);
      state++;
      break;

    case 1:
      //2 Bak C DC 100%---------------------------------------------------------? pwm motor3
      //3 Bak A, B, D DC 50%----------------------------------------------------? pwm motor124
      //4
      P("D2");
      int2byte(BakCRpm, 3);
      int2byte(BakDRpm, 3);
      Serial.write(12);
      delay(5000);
      state++;
      break;

    case 2:
      //5 hitung debit dr sensor ------------------------------------------------? baca sensor
      debit = 10; //dummy

      //6
      P("D3");
      Serial.write(12);
      //while !input kadar koagulan {
      while (digitalRead(3)) {} //dummy-------------------------------------------? baca keypad
      kadarKoa = 2000;    //dummy
      //}
      state++;
      break;

    case 3:
      //7
      volKoagulan = ((kadarKoa / 50) * debit) * 1000;
      timerBakC = 0;
      //8 TAMPILKAN NILAI VOLUME TERHITUNG     
      //9 aktifkan peristaltik B n detik--------------------------------------------? pompapb pwm
      //10 set timer 30 detik
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
      int2byte(volKoagulan, 4);
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
      while (digitalRead(3)) {} //dummy---------------------------------------------? timer jalan atau nunggu?
      dayaIkat = 2;    //dummy
      state++;
      break;

    case 8:
      //17 measure ph in ph sensor
      //hitng volLarDisinfect berdasar tabel
      P("D:");
      int2byte(ph / 10, 1);
      int2byte(ph % 10, 1);
      int2byte(volLarDisinfect, 1);
      Serial.write(12);
      delay(5000);
      //18 aktifkan peristaltik A n detik------------------------------------------? pompa PA,pwm
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
      delay(1000); //dummy
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
      state = 0;
      break;

    default:
      break;
  }
}

void int2byte(int val, byte digit) {
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

