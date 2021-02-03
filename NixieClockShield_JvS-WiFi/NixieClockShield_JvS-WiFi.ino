const String FirmwareVersion = "808080";
#define HardwareVersion "NCS314 for HW 2.x"

/* Adapted firmware for Gra&Afch NixieShield, HW v2.3
The main difference with the origional firmware:
- Added digit fading, numbers now fade-in & fade-out
- Used an Arduino Mega with ESP2866 WiFi built-in, sync with NTP Server
- Overall attempted to clean-up the code a bit
- Remove IR functionality as it will never be used and clutter the code (see Gra&Afch original FW if you want to see how to re-enable)
- Also remove GPS functionality as we now have WiFi NTP sync
*/

#include "NixieClockShield_JvS-Wifi.h"

byte data[12];
byte addr[8];
int celsius, fahrenheit;

bool RTC_present;

OneWire ds(pinTemp);
bool TempPresent = false;
bool TubesOn = true;

String stringToDisplay = "000000";              //Content of this string will be displayed on tubes (must be 6 chars length)
String prevstringToDisplay = "000000";          //addtl code for fade effect
bool newNumber;                                 //Do we start a new number (usually a new second)

byte dotPattern = B00000000; //bit mask for separeting dots (1 - on, 0 - off)
//B10000000 - upper dots
//B01000000 - lower dots


byte zero = 0x00; //workaround for issue #527
int RTC_hours, RTC_minutes, RTC_seconds, RTC_day, RTC_month, RTC_year, RTC_day_of_week;

bool BlinkUp = false;
bool BlinkDown = false;
bool RGBLedsOn = false;  // set from EEPROM later

Tone tone1;
#define isdigit(n) (n >= '0' && n <= '9')
//char *song = "MissionImp:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d";
char *song = "PinkPanther:d=4,o=5,b=160:8d#,8e,2p,8f#,8g,2p,8d#,8e,16p,8f#,8g,16p,8c6,8b,16p,8d#,8e,16p,8b,2a#,2p,16a,16g,16e,16d,2e";
//char *song="VanessaMae:d=4,o=6,b=70:32c7,32b,16c7,32g,32p,32g,32p,32d#,32p,32d#,32p,32c,32p,32c,32p,32c7,32b,16c7,32g#,32p,32g#,32p,32f,32p,16f,32c,32p,32c,32p,32c7,32b,16c7,32g,32p,32g,32p,32d#,32p,32d#,32p,32c,32p,32c,32p,32g,32f,32d#,32d,32c,32d,32d#,32c,32d#,32f,16g,8p,16d7,32c7,32d7,32a#,32d7,32a,32d7,32g,32d7,32d7,32p,32d7,32p,32d7,32p,16d7,32c7,32d7,32a#,32d7,32a,32d7,32g,32d7,32d7,32p,32d7,32p,32d7,32p,32g,32f,32d#,32d,32c,32d,32d#,32c,32d#,32f,16c";
//char *song="DasBoot:d=4,o=5,b=100:d#.4,8d4,8c4,8d4,8d#4,8g4,a#.4,8a4,8g4,8a4,8a#4,8d,2f.,p,f.4,8e4,8d4,8e4,8f4,8a4,c.,8b4,8a4,8b4,8c,8e,2g.,2p";
//char *song="Scatman:d=4,o=5,b=200:8b,16b,32p,8b,16b,32p,8b,2d6,16p,16c#.6,16p.,8d6,16p,16c#6,8b,16p,8f#,2p.,16c#6,8p,16d.6,16p.,16c#6,16b,8p,8f#,2p,32p,2d6,16p,16c#6,8p,16d.6,16p.,16c#6,16a.,16p.,8e,2p.,16c#6,8p,16d.6,16p.,16c#6,16b,8p,8b,16b,32p,8b,16b,32p,8b,2d6,16p,16c#.6,16p.,8d6,16p,16c#6,8b,16p,8f#,2p.,16c#6,8p,16d.6,16p.,16c#6,16b,8p,8f#,2p,32p,2d6,16p,16c#6,8p,16d.6,16p.,16c#6,16a.,16p.,8e,2p.,16c#6,8p,16d.6,16p.,16c#6,16a,8p,8e,2p,32p,16f#.6,16p.,16b.,16p.";
//char *song="Popcorn:d=4,o=5,b=160:8c6,8a#,8c6,8g,8d#,8g,c,8c6,8a#,8c6,8g,8d#,8g,c,8c6,8d6,8d#6,16c6,8d#6,16c6,8d#6,8d6,16a#,8d6,16a#,8d6,8c6,8a#,8g,8a#,c6";
//char *song="WeWishYou:d=4,o=5,b=200:d,g,8g,8a,8g,8f#,e,e,e,a,8a,8b,8a,8g,f#,d,d,b,8b,8c6,8b,8a,g,e,d,e,a,f#,2g,d,g,8g,8a,8g,8f#,e,e,e,a,8a,8b,8a,8g,f#,d,d,b,8b,8c6,8b,8a,g,e,d,e,a,f#,1g,d,g,g,g,2f#,f#,g,f#,e,2d,a,b,8a,8a,8g,8g,d6,d,d,e,a,f#,2g";
#define OCTAVE_OFFSET 0
char *p;

int notes[] = { 0,
                NOTE_C4, NOTE_CS4, NOTE_D4, NOTE_DS4, NOTE_E4, NOTE_F4, NOTE_FS4, NOTE_G4, NOTE_GS4, NOTE_A4, NOTE_AS4, NOTE_B4,
                NOTE_C5, NOTE_CS5, NOTE_D5, NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_AS5, NOTE_B5,
                NOTE_C6, NOTE_CS6, NOTE_D6, NOTE_DS6, NOTE_E6, NOTE_F6, NOTE_FS6, NOTE_G6, NOTE_GS6, NOTE_A6, NOTE_AS6, NOTE_B6,
                NOTE_C7, NOTE_CS7, NOTE_D7, NOTE_DS7, NOTE_E7, NOTE_F7, NOTE_FS7, NOTE_G7, NOTE_GS7, NOTE_A7, NOTE_AS7, NOTE_B7
              };

int fireforks[] = {0, 0, 1, //1
                   -1, 0, 0, //2
                   0, 1, 0, //3
                   0, 0, -1, //4
                   1, 0, 0, //5
                   0, -1, 0
                  }; //array with RGB rules (0 - do nothing, -1 - decrese, +1 - increse

void setRTCDateTime(byte h, byte m, byte s, byte d, byte mon, byte y, byte w = 1);

bool LEDsLock = false;

//special transaction handling
bool transactionInProgress = false; //antipoisoning or startup-test transaction in progress iow a 'special' situation. doIndication needs to know!


extern const int LEDsDelay;

/*******************************************************************************************************
  Init Programm
*******************************************************************************************************/
void setup()
{
  Wire.begin();
  //setRTCDateTime(23,40,00,25,7,15,1);

  Serial.begin(115200);
  Serial3.begin(115200);              //this is the interface to the ESP8622 builtin WiFi

  delay(3000);

  init_EEPROM();

  Serial.print(F("led lock="));
  Serial.println(LEDsLock);

  pinMode(RedLedPin, OUTPUT);
  pinMode(GreenLedPin, OUTPUT);
  pinMode(BlueLedPin, OUTPUT);

  tone1.begin(pinBuzzer);
  song = parseSong(song);

  pinMode(LEpin, OUTPUT);

  // SPI setup
  SPISetup();
  LEDsSetup();
  buttonSetup();

  pinMode(pinBuzzer, OUTPUT);

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  doTest();
  testDS3231TempSensor();
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  if (LEDsLock == 1)
  {
    setLEDsFromEEPROM();
  }
  getRTCTime();
  byte prevSeconds = RTC_seconds;
  unsigned long RTC_ReadingStartTime = millis();
  RTC_present = true;
  while (prevSeconds == RTC_seconds)
  {
    getRTCTime();
    //Serial.println(RTC_seconds);
    if ((millis() - RTC_ReadingStartTime) > 3000)
    {
      Serial.println(F("Warning! RTC not responding"));
      RTC_present = false;
      break;
    }
  }
  setTime(RTC_hours, RTC_minutes, RTC_seconds, RTC_day, RTC_month, RTC_year);
}

int rotator = 0; //index in array with RGB "rules" (increse by one on each 255 cycles)
int cycle = 0; //cycles counter
int RedLight = 255;
int GreenLight = 0;
int BlueLight = 0;
unsigned long prevTime = 0; // time of lase tube was lit
unsigned long prevTime4FireWorks = 0; //time of last RGB changed

/***************************************************************************************************************
  MAIN Programm
***************************************************************************************************************/
void loop() {

  if (((millis() % 10000) == 0) && (RTC_present)) //synchronize with RTC every 10 seconds
  {
    getRTCTime();
    setTime(RTC_hours, RTC_minutes, RTC_seconds, RTC_day, RTC_month, RTC_year);
    //Serial.println(F("Sync"));
  }

  GetNTPData();                                 //synchronize with NTP Server, default is once per minute
  if ((millis() - prev_ntp_sync > NTP_SYNC_INTERVAL) && ntp_sync_pending)
  {

    Serial.print("[*] Sync from NTP server: ");
    Serial.println(ntp_buffer);

    ntp_rtcupdate();

    ntp_sync_pending = false;
    prev_ntp_sync = millis();
  }

  p = playmusic(p);

  if ((millis() - prevTime4FireWorks) > LEDsDelay)
  {
    rotateFireWorks(); //change color (by 1 step)
    prevTime4FireWorks = millis();
  }

  if ((menuPosition == TIMEINDEX) || (modeChanger == false) ) slotMachine();

  handleButtons();
  doIndication();

  if (editMode == false && modeChanger == false & transactionInProgress == false) {       //Everything is normal --> tik-tok
    stringToDisplay = updateTimeString();
  }

  checkAlarmTime();
  checkTubeTimers();
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// End of main loop()


void init_EEPROM()
{
  // Initialize settings from EEPROM
  if (EEPROM.read(RGB_EEPROM_ADDR) != 0) { RGBLedsOn = true; } else { RGBLedsOn = false; }
  if (EEPROM.read(HOURFORMAT_EEPROM_ADDR) != 12) { storedValues[T_MODE] = 24; } else { storedValues[T_MODE] = 12; }
  if (EEPROM.read(ALARMTIME_EEPROM_ADDR) > 24) { storedValues[A_HOUR] = 0; } else { storedValues[A_HOUR] = EEPROM.read(ALARMTIME_EEPROM_ADDR); }
  if (EEPROM.read(ALARMTIME_EEPROM_ADDR + 1) > 60) { storedValues[A_MINUTE] = 0; } else { storedValues[A_MINUTE] = EEPROM.read(ALARMTIME_EEPROM_ADDR + 1); }
  if (EEPROM.read(ALARMTIME_EEPROM_ADDR + 2) > 60) { storedValues[A_SECOND] = 0; } else { storedValues[A_SECOND] = EEPROM.read(ALARMTIME_EEPROM_ADDR + 2); }
  if (EEPROM.read(ALARMARMED_EEPROM_ADDR) > 1) { storedValues[A_ON] = 0; } else { storedValues[A_ON] = EEPROM.read(ALARMARMED_EEPROM_ADDR); }
  if (EEPROM.read(LEDLOCK_EEPROM_ADDR) > 1) { LEDsLock = false; } else { LEDsLock = EEPROM.read(LEDLOCK_EEPROM_ADDR); }
  if (EEPROM.read(DATEFORMAT_EEPROM_ADDR) > 1) { storedValues[D_FORMAT] = 0; } else { storedValues[D_FORMAT] = EEPROM.read(DATEFORMAT_EEPROM_ADDR); }

  // Tube timer EEPROM values
  if (EEPROM.read(TT_OFF_HH_EEPROM_ADDR) > 24) { storedValues[TOFF_HOUR] = 0; } else { storedValues[TOFF_HOUR] = EEPROM.read(TT_OFF_HH_EEPROM_ADDR); }
  if (EEPROM.read(TT_OFF_MM_EEPROM_ADDR) > 60) { storedValues[TOFF_MINUTE] = 0; } else { storedValues[TOFF_MINUTE] = EEPROM.read(TT_OFF_MM_EEPROM_ADDR); }
  if (EEPROM.read(TT_ON_HH_EEPROM_ADDR) > 24) { storedValues[TON_HOUR] = 0; } else { storedValues[TON_HOUR] = EEPROM.read(TT_ON_HH_EEPROM_ADDR); }
  if (EEPROM.read(TT_ON_MM_EEPROM_ADDR) > 60) { storedValues[TON_MINUTE] = 0; } else { storedValues[TON_MINUTE] = EEPROM.read(TT_ON_MM_EEPROM_ADDR); }
  if (EEPROM.read(TT_ENABLE_EEPROM_ADDR) > 1) { storedValues[TT_ON] = 1; } else { storedValues[TT_ON] = EEPROM.read(TT_ENABLE_EEPROM_ADDR); }

  randomSeed(hour() + minute() + second());

  Serial.println("Fromn EEPROM:" + String(storedValues[TT_ON]));

}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// End of init_EEPROM()

void checkTubeTimers()
{
  static bool TubeTimer_MinuteBlock = false;          // Block seconds to stop trigggering
  static unsigned long lastTime_TT_Triggired = 0;
  static bool          ledState = RGBLedsOn; 

  if (storedValues[TT_ON] == 1)
  {
    if (TubeTimer_MinuteBlock == true)
    { 
      // Reset Second block after a second has elapsed, else exit now
      if ((millis() - lastTime_TT_Triggired) > 60100) { TubeTimer_MinuteBlock = false; }
      else { return; }
    }

    // Check OFF timer (do not turn off tubes while in editMode)
    if ((hour() == storedValues[TOFF_HOUR]) && (minute() == storedValues[TOFF_MINUTE] && !editMode))
    {
      lastTime_TT_Triggired = millis();
      TubeTimer_MinuteBlock = true;

      if (TubesOn) {
        ledState = RGBLedsOn;  // we're switching off, save state of the leds
        RGBLedsOn = false;
      }

      TubesOn = false;
    
      #ifdef DEBUG
      Serial.println("[!] TubeTimer OFF Triggered");
      #endif
    }

    // Check ON timer
    else if ((hour() == storedValues[TON_HOUR]) && (minute() == storedValues[TON_MINUTE]))
    {
      lastTime_TT_Triggired = millis();
      TubeTimer_MinuteBlock = true;

      if (!TubesOn) {
        RGBLedsOn = ledState;  // we're switching on, restore state of the leds
        if (RGBLedsOn) {
          EEPROM.write(RGB_EEPROM_ADDR, 1);
          analogWrite(RedLedPin, EEPROM.read(LEDREDVAL_EEPROM_ADDR));
          analogWrite(GreenLedPin, EEPROM.read(LEDGREENVAL_EEPROM_ADDR));
          analogWrite(BlueLedPin, EEPROM.read(LEDBLUEVAL_EEPROM_ADDR));
        }
      }

      TubesOn = true;
    
      #ifdef DEBUG
      Serial.println("[!] TubeTimer ON Triggered");
      #endif
    }
  }
}
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// End of checkTubeTimers()

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Start NTP code

void GetNTPData()
{
  if (Serial3.available() > 0) {
    byte ntp_in_byte = Serial3.read();
    Serial.print(char(ntp_in_byte));
    switch (ntp_in_byte)
    {
      case 0x0A:  // Newline charactar
        ntp_buffer[ntp_position] = 0x00;
        ntp_position = 0;
        ntp_sync_pending = true;
        break;
      
      case 'E':

        Serial.println("[!] Error While parsing NTP string");

        ntp_position = 0;
        ntp_sync_pending = false;
        break;
      
      case 'U':
        // Skip
        break;

      case ' ':
        // Skip
        break;

      default:
        ntp_buffer[ntp_position] = ntp_in_byte;
        ntp_position++;
        break;
    } // end switch
  } // end if
}

void ntp_rtcupdate()
{
  // Convert UNIX timestamp to lastTime_TT_Triggired
  unsigned long utc_time = atol(ntp_buffer);
  unsigned long local_time = CE.toLocal(utc_time, &tcr);

  // Update the system time
  
  if (year(local_time) != 1970) {                                 //Make sure we didn't get a BS time string -- it happens
    setTime(hour(local_time), minute(local_time), second(local_time), day(local_time), month(local_time), year(local_time) % 1000);

    Serial.print("[i] UTC Time: ");
    Serial.print(utc_time);
    Serial.print(" Local Time: ");
    Serial.print(local_time);
    Serial.print(" > ");
    Serial.print(hour(local_time));
    Serial.print(":");
    Serial.print(minute(local_time));
    Serial.print(":");
    Serial.print(second(local_time));
    Serial.print("  Date: ");
    Serial.print(month(local_time));
    Serial.print("/");
    Serial.print(day(local_time));
    Serial.print("/");
    Serial.println(year(local_time));

    // Update the RTC
    setRTCDateTime(hour(), minute(), second(), day(), month(), year() % 1000, 1);
    prevstringToDisplay = stringToDisplay;
    stringToDisplay = getTimeNow();
    Serial.println("Clock updated with NTP time");
  }
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// End of NTP code
// ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Start of slotmachine code

void slotMachine()
{
  static unsigned long timeToNextStep = SLOTMACHINEBETWEENTIME;
  static unsigned long timeOfLastStep = millis();
  static unsigned long lastSpin;
  static int           spinCounter, spinSpeed, slowDown, slowest;
  static int           stepCounter = 0;                // step 1) morph to date step 2) stay in date for x period step 3) morph back to time
  static bool          spinInProgress = false;
  static bool          newRound = false;
  static int           f_d[6], t_d[6], iterations_d[6], speed[6];

  if (editMode || modeChanger) return;                 //if user is fiddling with the buttons not going to do this

  if( ((millis() - timeOfLastStep) > timeToNextStep) && !transactionInProgress) {    //Execute the next step in the slotmachine process
    stepCounter = 1;                                                                  //new slotspin, goto stepCounter = 1
    newRound = true;
    spinInProgress = true;
    transactionInProgress = true;
  } //end of execute next step

  if (transactionInProgress) {
    if (spinInProgress) {
      //either spinning up or down (case 1 or 2)
      switch (stepCounter) {
        case 1:                                       //morph to date
          if (newRound) {                             //determine digits we want to spin from and to
            slowDown = 50;                            
            spinCounter = 20;                         //minimum 20 spins
            slowest = 6;                              //max # of pauses between digits, more = too slow. Works in conjunction with the millis() check. Tune to personal preference
            String cDateTmp = preZero(day()) + preZero(month()) + preZero(year()-2000);
            for (int i=0; i<6; i++) {
              f_d[i] = (stringToDisplay.substring(i,i+1)).toInt();
              t_d[i] = (cDateTmp.substring(i,i+1)).toInt();
              //iterations_d[i] = (spincounter + (i*20) + (t_d[i] - f_d[i]));       //how many times, spincounter +10 extra per sigit + difference
              iterations_d[i] = (spinCounter + (slowDown*i) + (t_d[i] - f_d[i]));
              speed[i] = 0;
            }
            newRound=false;
            lastSpin = millis();
          }

          if ((millis() - lastSpin) > 50) {                            //up the digits
            for (int i=0; i<6; i++) {
              if (iterations_d[i] < 10 && iterations_d[i] > 0) {    //need to slow down for this digit - last 10 iterations
                if (speed[i] >= (10 - iterations_d[i]) || speed[i] >= slowest) {
                  f_d[i]++; if (f_d[i] > 9) f_d[i]=0;
                  iterations_d[i]--;
                  speed[i] = 0;
                } else speed[i]++;                                 //no new number yet, slowing down
              } else {                                            //At full speed or full stop
                if (iterations_d[i] != 0) {
                  iterations_d[i]--;
                  f_d[i]++;  if (f_d[i] > 9) f_d[i]=0;
                }
              }
            }
            //            if (spinSpeed < 110) spinSpeed++;
            stringToDisplay = preZero((f_d[0]*10) + f_d[1]) + preZero((f_d[2]*10) + f_d[3]) + preZero((f_d[4]*10) + f_d[5]);
            if (iterations_d[5] == 0) {                        //done spinning last digit
              stepCounter = 2;
              timeOfLastStep = millis();
              timeToNextStep = 3000;
              spinInProgress = false;
            }
            lastSpin = millis();
          }
        break;
        case 3:
          //Here we do the spin down
          transactionInProgress = false;
          timeToNextStep = random(60000, SLOTMACHINEBETWEENTIME);
          //timeToNextStep = SLOTMACHINEBETWEENTIME;
          timeOfLastStep = millis();
        break;
      }  // end switch (dit kan handiger)
    } else {
      //we are in step 2, showing the date
      if (millis() - timeOfLastStep > timeToNextStep) {
        stepCounter = 3;                                        //stepcounter will now be 3 --> spin down
        spinInProgress = true;
        newRound = true;
      }
    }
  }
} //end void slotMachine()
// End of slotmachine code
// ------------------------------------------------------------------------------------------------------


String preZero(int digit)
{
  digit = abs(digit);
  if (digit < 10) return String("0") + String(digit);
  else return String(digit);
}

String updateTimeString()
{
  static int prevS = -1;

  if (second() != prevS)
  {
    prevstringToDisplay = stringToDisplay;
    newNumber=true;
    prevS = second();
    return getTimeNow();
  } else return stringToDisplay;
}

String getTimeNow()
{
  if (storedValues[T_MODE] == 24) return preZero(hour()) + preZero(minute()) + preZero(second());
   else return preZero(hourFormat12()) + preZero(minute()) + preZero(second());
}

void doTest()
{
  transactionInProgress = true;
  Serial.print(F("Firmware version: "));
  Serial.println(FirmwareVersion.substring(1, 2) + "." + FirmwareVersion.substring(2, 5));
  Serial.println(HardwareVersion);
  Serial.println(F("Start Test"));

  //p = song;
  //parseSong(p);
  //p=0; //need to be deleted

  LEDsTest();

#ifdef tubes8
  String testStringArray[11] = {"00000000", "11111111", "22222222", "33333333", "44444444", "55555555", "66666666", "77777777", "88888888", "99999999", ""};
  testStringArray[10] = FirmwareVersion + "00";
#endif
#ifdef tubes6
  String testStringArray[11] = {"000000", "111111", "222222", "333333", "444444", "555555", "666666", "777777", "888888", "999999", ""};
  testStringArray[10] = FirmwareVersion;
#endif

  int dlay = 500;
  bool test = 1;
  byte strIndex = -1;
  unsigned long startOfTest = millis() + 1000; //disable delaying in first iteration
  bool digitsLock = false;
  while (test)
  {
    if (digitalRead(DOWN_PIN) == 0) digitsLock = true;
    if (digitalRead(UP_PIN) == 0) digitsLock = false;

    if ((millis() - startOfTest) > dlay)
    {
      startOfTest = millis();
      if (!digitsLock) strIndex = strIndex + 1;
      if (strIndex == 10) dlay = 2000;
      if (strIndex > 10) {
        test = false;
        strIndex = 10;
      }

      stringToDisplay = testStringArray[strIndex];
      Serial.println(stringToDisplay);
      doIndication();
    }
    delayMicroseconds(2000);
  };

  if ( !ds.search(addr))
  {
    Serial.println(F("Temp. sensor not found."));
  } else TempPresent = true;

  testDS3231TempSensor();

  Serial.println(F("Stop Test"));
  transactionInProgress = false;
}

void doDotBlink()
{
  //dotPattern = B11000000; return; //always on
  //dotPattern = B00000000; return; //always off
  if (second() % 2 == 0) dotPattern = B11000000;
  else dotPattern = B00000000;
}

void setRTCDateTime(byte h, byte m, byte s, byte d, byte mon, byte y, byte w)
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero); //stop Oscillator

  Wire.write(decToBcd(s));
  Wire.write(decToBcd(m));
  Wire.write(decToBcd(h));
  Wire.write(decToBcd(w));
  Wire.write(decToBcd(d));
  Wire.write(decToBcd(mon));
  Wire.write(decToBcd(y));

  Wire.write(zero); //start

  Wire.endTransmission();

}

byte decToBcd(byte val) {
  // Convert normal decimal numbers to binary coded decimal
  return ( (val / 10 * 16) + (val % 10) );
}

byte bcdToDec(byte val)  {
  // Convert binary coded decimal to normal decimal numbers
  return ( (val / 16 * 10) + (val % 16) );
}

void getRTCTime()
{
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(zero);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);

  RTC_seconds = bcdToDec(Wire.read());
  RTC_minutes = bcdToDec(Wire.read());
  RTC_hours = bcdToDec(Wire.read() & 0b111111); //24 hour time
  RTC_day_of_week = bcdToDec(Wire.read()); //0-6 -> sunday - Saturday
  RTC_day = bcdToDec(Wire.read());
  RTC_month = bcdToDec(Wire.read());
  RTC_year = bcdToDec(Wire.read());
}

int extractDigits(byte b)
{
  String tmp = "1";

  if (b == B00000011)
  {
    tmp = stringToDisplay.substring(0, 2);
  }
  if (b == B00001100)
  {
    tmp = stringToDisplay.substring(2, 4);
  }
  if (b == B00110000)
  {
    tmp = stringToDisplay.substring(4);
  }
  return tmp.toInt();
}

void injectDigits(byte b, int value)
{
  if (b == B00000011) stringToDisplay = preZero(value) + stringToDisplay.substring(2);
  if (b == B00001100) stringToDisplay = stringToDisplay.substring(0, 2) + preZero(value) + stringToDisplay.substring(4);
  if (b == B00110000) stringToDisplay = stringToDisplay.substring(0, 4) + preZero(value);
}

bool isValidDate()
{
  int days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (storedValues[D_YEAR] % 4 == 0) days[1] = 29;
  if (storedValues[D_DAY] > days[storedValues[D_MONTH] - 1]) return false;
  else return true;

}

byte default_dur = 4;
byte default_oct = 6;
int bpm = 63;
int num;
long wholenote;
long duration;
byte note;
byte scale;
char* parseSong(char *p)
{
  // Absolutely no error checking in here
  // format: d=N,o=N,b=NNN:
  // find the start (skip name, etc)

  while (*p != ':') p++;   // ignore name
  p++;                     // skip ':'

  // get default duration
  if (*p == 'd')
  {
    p++; p++;              // skip "d="
    num = 0;
    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    if (num > 0) default_dur = num;
    p++;                   // skip comma
  }

  // get default octave
  if (*p == 'o')
  {
    p++; p++;              // skip "o="
    num = *p++ - '0';
    if (num >= 3 && num <= 7) default_oct = num;
    p++;                   // skip comma
  }

  // get BPM
  if (*p == 'b')
  {
    p++; p++;              // skip "b="
    num = 0;
    while (isdigit(*p))
    {
      num = (num * 10) + (*p++ - '0');
    }
    bpm = num;
    p++;                   // skip colon
  }

  // BPM usually expresses the number of quarter notes per minute
  wholenote = (60 * 1000L / bpm) * 4;  // this is the time for whole note (in milliseconds)
  return p;
}

// now begin note loop
static unsigned long lastTimeNotePlaying = 0;
char* playmusic(char *p)
{
  if (*p == 0)
  {
    return p;
  }
  if (millis() - lastTimeNotePlaying > duration)
    lastTimeNotePlaying = millis();
  else return p;
  // first, get note duration, if available
  num = 0;
  while (isdigit(*p))
  {
    num = (num * 10) + (*p++ - '0');
  }

  if (num) duration = wholenote / num;
  else duration = wholenote / default_dur;  // we will need to check if we are a dotted note after

  // now get the note
  note = 0;

  switch (*p)
  {
    case 'c':
      note = 1;
      break;
    case 'd':
      note = 3;
      break;
    case 'e':
      note = 5;
      break;
    case 'f':
      note = 6;
      break;
    case 'g':
      note = 8;
      break;
    case 'a':
      note = 10;
      break;
    case 'b':
      note = 12;
      break;
    case 'p':
    default:
      note = 0;
  }
  p++;

  // now, get optional '#' sharp
  if (*p == '#')
  {
    note++;
    p++;
  }

  // now, get optional '.' dotted note
  if (*p == '.')
  {
    duration += duration / 2;
    p++;
  }

  // now, get scale
  if (isdigit(*p))
  {
    scale = *p - '0';
    p++;
  }
  else
  {
    scale = default_oct;
  }

  scale += OCTAVE_OFFSET;

  if (*p == ',')
    p++;       // skip comma for next note (or we may be at the end)

  // now play the note

  if (note)
  {
    tone1.play(notes[(scale - 4) * 12 + note], duration);
    if (millis() - lastTimeNotePlaying > duration)
      lastTimeNotePlaying = millis();
    else return p;
    tone1.stop();
  }
  else
  {
    return p;
  }
  Serial.println(F("Incorrect Song Format!"));
  return 0; //error
}


bool Alarm1SecondBlock = false;
unsigned long lastTimeAlarmTriggired = 0;
void checkAlarmTime()
{
  if (storedValues[A_ON] == 0) return;
  if ((Alarm1SecondBlock == true) && ((millis() - lastTimeAlarmTriggired) > 1000)) Alarm1SecondBlock = false;
  if (Alarm1SecondBlock == true) return;
  if ((hour() == storedValues[A_HOUR]) && (minute() == storedValues[A_MINUTE]) && (second() == storedValues[A_SECOND]))
  {
    lastTimeAlarmTriggired = millis();
    Alarm1SecondBlock = true;
    Serial.println(F("Wake up, Neo!"));
    p = song;
  }
}


void testDS3231TempSensor()
{
  int8_t DS3231InternalTemperature = 0;
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x11);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 2);
  DS3231InternalTemperature = Wire.read();
  Serial.print(F("DS3231_T="));
  Serial.println(DS3231InternalTemperature);
  if ((DS3231InternalTemperature < 5) || (DS3231InternalTemperature > 60))
  {
    Serial.println(F("Faulty DS3231!"));
    for (int i = 0; i < 5; i++)
    {
      tone1.play(1000, 1000);
      delay(2000);
    }
  }
}
