void setup()
{
  pinMode(14, OUTPUT);  // green LED, LOW == ON
  pinMode(15, OUTPUT);  // red LED, LOW == ON
  digitalWrite(14, HIGH);  // turn green LED off
  digitalWrite(15, HIGH);  // turn red LED off
}

void loop()
{
  digitalWrite(15, LOW);   // turn red LED on
  delay(1000);             // wait for a second
  digitalWrite(14, LOW);   // turn green LED on
  delay(1000);             // wait for a second
  digitalWrite(15, HIGH);  // turn red LED off
  delay(1000);             // wait for a second
  digitalWrite(14, HIGH);  // turn green LED off
  delay(1000);             // wait for a second
}

