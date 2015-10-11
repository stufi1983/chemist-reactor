//ATMega8535 8 MHz
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 13; // Analog output pin that the LED is attached to

int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)

void setup() {
  Serial.begin(300);
}

void loop() {
  sensorValue = analogRead(analogInPin);
  outputValue = map(sensorValue, 0, 1023, 0, 255);
  analogWrite(analogOutPin, outputValue);

  Serial.print(("sensor = " ));
  Serial.print(sensorValue);
  Serial.write(12);

  delay(1000);
  analogWrite(analogOutPin, 0);
  delay(1000);
}
