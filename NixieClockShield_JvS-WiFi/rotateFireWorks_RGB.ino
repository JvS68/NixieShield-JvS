#define LEDsSpeed      1
const int LEDsDelay=5;

void LEDsSetup(){}

void rotateFireWorks()
{
  if (tone1.isPlaying()) return;
  if (!RGBLedsOn)
  {
    analogWrite(RedLedPin, 0 );
    analogWrite(GreenLedPin, 0);
    analogWrite(BlueLedPin, 0);
    return;
  }
  if (LEDsLock) return;
  RedLight = RedLight + fireforks[rotator * 3];
  GreenLight = GreenLight + fireforks[rotator * 3 + 1];
  BlueLight = BlueLight + fireforks[rotator * 3 + 2];
  analogWrite(RedLedPin, RedLight );
  analogWrite(GreenLedPin, GreenLight);
  analogWrite(BlueLedPin, BlueLight);
  // ********
  //Serial.print(F("RED="));
  //Serial.println(RedLight);
  //Serial.print(F("GREEN="));
  //Serial.println(GreenLight);
  //Serial.print(F("Blue="));
  //Serial.println(BlueLight);
  // ********
  
  cycle = cycle + 1;
  if (cycle == 255)
  {
    rotator = rotator + 1;
    cycle = 0;
  }
  if (rotator > 5) rotator = 0;
}

void LEDsTest()
{
  analogWrite(RedLedPin,255);
  delay(1000);
  analogWrite(RedLedPin,0);
  analogWrite(GreenLedPin,255);
  delay(1000);
  analogWrite(GreenLedPin,0);
  analogWrite(BlueLedPin,255);
  delay(1000); 
  analogWrite(BlueLedPin,0);
}

void setLEDsFromEEPROM()
{
  int R,G,B;
  R=EEPROM.read(LEDREDVAL_EEPROM_ADDR);
  G=EEPROM.read(LEDGREENVAL_EEPROM_ADDR);
  B=EEPROM.read(LEDBLUEVAL_EEPROM_ADDR);

  analogWrite(RedLedPin, EEPROM.read(LEDREDVAL_EEPROM_ADDR));
  analogWrite(GreenLedPin, EEPROM.read(LEDGREENVAL_EEPROM_ADDR));
  analogWrite(BlueLedPin, EEPROM.read(LEDBLUEVAL_EEPROM_ADDR));
    
  /*
  Serial.println(F("Readed from EEPROM"));
  Serial.print(F("RED="));
  Serial.println(R);
  Serial.print(F("GREEN="));
  Serial.println(G);
  Serial.print(F("Blue="));
  Serial.println(B);
  */
}
