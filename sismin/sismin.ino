#define PL(x) {Serial.println(x);  Serial.write(12);}
#define P(x) {Serial.print(x);}
#define L() Serial.write(12);}
#define W(x) Serial.write(x);}

//ATMega8535 8 MHz
int BakDRpm = 100;
void setup() {
  Serial.begin(300);
  PL("D1");
  delay(5000);
}
byte int2bytebuf[5];

void int2byte(int val, byte digit){
  //satuan
  int2bytebuf[4] = (val % 10) + 0x30;
  if(val<9){int2bytebuf[3]=0x20;}else{int2bytebuf[3] = ( ( (val / 10) % 10) + 0x30);}
  if(val<99){int2bytebuf[2]=0x20;}else{int2bytebuf[2] = ( ( (val / 100) % 10) + 0x30);}
  if(val<999){int2bytebuf[1]=0x20;}else{int2bytebuf[1] = ( ( (val / 1000) % 10) + 0x30);}
  if(val<9999){int2bytebuf[0]=0x20;}else{int2bytebuf[0] = ( ( (val / 10000) % 10) + 0x30);}
  for(byte i=5-digit; i<5; i++){ Serial.write(int2bytebuf[i]);}
}

void loop() {
  P("D2"); 
  int2byte(BakDRpm,3); 
  int2byte(BakDRpm,3); 
  Serial.write(12); 
  delay(1000);
  BakDRpm++;
}
