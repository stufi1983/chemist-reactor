//ATMega8535 8 MHz 
//
const byte rows = 4;           //number of rows of keypad
const byte columns = 4;          //number of columnss of keypad
const byte Output[rows] = {23, 22, 21, 20}; //array of pins used as output for rows of keypad
const byte Input[columns] = {29, 28, 27, 26}; //array of pins used as input for columnss of keypad

#define FLOWINTERRUPT   //koneksi flow sensor,pilih: FLOWINTERRUPT di pin INT0/16 atau FLOWDIGITAL di pin PA1/39

//NB: urutan pin header untukflow sensor di PCB tidak pas dengan sensor
#define NOTCEKFLOW         //test Flow Sensor, pilih: CEKFLOW untuk test atau NOTCEKFLOW untuk normal
#define NOTCEKPH           //test ph meter, pilih: CEKPH untuk test atau NOTCEKPH untuk normal
#define CEKKP           //test keypad
#define NOTCEKPOT          //test potensio pengatur putaran
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
#define MOTORB    22 //PC6
#define MOTORC    12 //PD4  --->HARUSNYA DI OUTPUT TIMER
#define POTMOTORC A7
#define MOTORD    13 //PD5  --->HARUSNYA DI OUTPUT TIMER
#define POTMOTORD A6

#define PERISA    19 //PC3
#define PERISB    18 //PC2

#define MAXDIGIT  6

#define DEBITPERISB 10 //10 mL/detik
#define DEBITPERISA 10 //10 mL/detik

#define COL   16

byte bytes[COL];
byte byteNum = 0;
byte int2bytebuf[MAXDIGIT];

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
  state = 2;
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

  for (byte i = 0; i < rows; i++) //for loop used to make pin mode of outputs as output
  {
    pinMode(Output[i], OUTPUT);
  }
  for (byte s = 0; s < columns; s++) //for loop used to makk pin mode of inputs as inputpullup
  {
    pinMode(Input[s], INPUT_PULLUP);
  }
}

int BakCRpm = 100;
int BakDRpm = 50;

int timerBakC = 0;
int timerBakD = 0; //30 minutes
int timerBakE = 0; //1 hour
int timerBakF = 0; //1 hour

void loop() {
  byte dgt = 0;
  int potCValue = 0;
  int potDValue = 0;



  const unsigned long period = 50; //little period used to prevent error
  unsigned long kdelay = 0;      // variable used in non-blocking delay


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
      beep(100);
      keypadEntry = false;
      while (!keypadEntry)
      {
        //2 Bak C DC 100%
        potCValue = analogRead(POTMOTORC);
        analogWrite(MOTORC, map(potCValue, 0, 1023, 0, 255));
        BakCRpm = potCValue / 10; if (BakCRpm > 100) BakCRpm = 100;

        //3 Bak D 50%
        potDValue = analogRead(POTMOTORD);
        analogWrite(MOTORC, map(potDValue, 0, 1023, 0, 255));
        BakDRpm = potDValue / 10; if (BakDRpm > 100) BakDRpm = 100;

        //4 Tampil kondisi Motor Pengaduk
        P("D2");
        int2byte(BakCRpm, 3);
        int2byte(BakDRpm, 3);
        Serial.write(12);
        delay(500); //eliminate LCD blinking

        if (millis() - kdelay > period) //used to make non-blocking delay
        {
          kdelay = millis(); //capture time from millis function
          if (keypad() < 50) { //press any key
            beep(100);
            delay(1000);
            keypadEntry = true;
          }
        }
      }
#ifdef NOTCEKPOT
      state++;
#endif
      delay(1000);
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
      //kadarKoa = 0;
      keypadEntry = false;
      //dgt = 0;
      while (!keypadEntry)
      {
        // kadarKoa = 4999;                                   //dummy, baca keypad  ---------
        if (millis() - kdelay > period) //used to make non-blocking delay
        {
          kdelay = millis(); //capture time from millis function
          byte kp = keypad();
          if (kp == 12) {
            kadarKoa = 0;  //*
            dgt = 0;
          }
          else if (kp == 14) {
            keypadEntry = true; //*
          }
          else if (kp < 50) { //press any key
            if (kadarKoa <= 999) {
              beep(12);
              if (dgt == 1)
              {
                kadarKoa = kadarKoa * 10 + mapkp(kp);
                dgt = 0;
              }
              else
                kadarKoa = mapkp(kp);
              dgt++;
            } else {
              kadarKoa = 0;
              beep(120);
            }
          }


        }
#ifdef CEKKP
        P("D3"); int2byte(kadarKoa, 4);
        Serial.write(12);
        delay(50);
#endif
      }
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
      int2byte(debit, 4);
      Serial.write(12);
      //9 aktifkan peristaltik B n detik
      //dummy, delay berdasar volKoagulan / debitPB setiap detik, sementara 1000 ms  ---------
      digitalWrite(PERISB, HIGH); delay(volKoagulan / DEBITPERISB * 1000); digitalWrite(PERISB, LOW);
      //10 set timer 30 detik
      timerBakC = 0;
      beep(100);
      for (byte i = 0; i < 5; i++) {
        P("D4");
        int2byte(timerBakC, 2);
        int2byte(debit, 4);
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
      /******************
            //dayaIkat = 2;                                             //dummy, baca keypad  ---------
            keypadEntry = false;
            dgt = 0;
            while (!keypadEntry)
            {
              if (millis() - kdelay > period) //used to make non-blocking delay
              {
                kdelay = millis(); //capture time from millis function
                byte kp = keypad();
                if (kp == 12) {
                  dayaIkat = 0;  //*
                  dgt = 0;
                }
                else if (kp == 14) {
                  keypadEntry = true; //*
                }
                else if (kp < 50) { //press any key
                  beep(20);
                  if (dgt == 1)
                  {
                    dayaIkat = dayaIkat * 10 + mapkp(kp);
                    dgt = 0;
                  }
                  else
                    dayaIkat = mapkp(kp);
                  dgt++;
                }

              }
      #ifdef CEKKP
              P("D3"); int2byte(dayaIkat, 4);
              Serial.write(12);
              delay(50);
      #endif
            }
            *************************/

      P("D9"); int2byte(dayaIkat, 4);
      Serial.write(12);
      beep(100);
      delay(2000);
      state++;
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
      digitalWrite(PERISA, HIGH); delay(volLarDisinfect / DEBITPERISA * 1000); digitalWrite(PERISA, LOW);
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

byte keypad() // function used to detect which button is used
{
  byte h = 0, v = 0; //variables used in for loops
  static bool no_press_flag = 0; //static flag used to ensure no button is pressed
  for (byte x = 0; x < columns; x++) // for loop used to read all inputs of keypad to ensure no button is pressed
  {
    if (digitalRead(Input[x]) == HIGH); //read evry input if high continue else break;
    else
      break;
    if (x == (columns - 1)) //if no button is pressed
    {
      no_press_flag = 1;
      h = 0;
      v = 0;
    }
  }
  if (no_press_flag == 1) //if no button is pressed
  {
    for (byte r = 0; r < rows; r++) //for loop used to make all output as low
      digitalWrite(Output[r], LOW);
    for (h = 0; h < columns; h++) // for loop to check if one of inputs is low
    {
      if (digitalRead(Input[h]) == HIGH) //if specific input is remain high (no press on it) continue
        continue;
      else    //if one of inputs is low
      {
        for (v = 0; v < rows; v++) //for loop used to specify the number of row
        {
          digitalWrite(Output[v], HIGH);  //make specified output as HIGH
          if (digitalRead(Input[h]) == HIGH) //if the input that selected from first sor loop is change to high
          {
            no_press_flag = 0;              //reset the no press flag;
            for (byte w = 0; w < rows; w++) // make all outputs as low
              digitalWrite(Output[w], LOW);
            return v * 4 + h; //return number of button
          }
        }
      }
    }
  }
  return 50;
}

byte mapkp(byte kp) {

  switch (kp)  //switch used to specify which button
  {
    case 0:
      return 1;
      break;
    case 1:
      return 2;
      break;
    case 2:
      return 3;
      break;
    //    case 3:
    //      return 'A';
    //      break;
    case 4:
      return 4;
      break;
    case 5:
      return 5;
      break;
    case 6:
      return 6;
      break;
    //    case 7:
    //      return 'B';
    //      break;
    case 8:
      return 7;
      break;
    case 9:
      return 8;
      break;
    case 10:
      return 9;
      break;
    //    case 11:
    //      return 'C';
    //      break;
    case 12:
      return '*';
      break;
    case 13:
      return 0;
      break;
    case 14:
      return '#';
      break;
    //    case 15:
    //      return 'D';
    //      break;
    default:
      ;
  }
}
