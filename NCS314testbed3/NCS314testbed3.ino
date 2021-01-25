
//NIXIE CLOCK SHIELD NCS314 v 2.x by GRA & AFCH (fominalec@gmail.com)
//test sketch to test principle of nmbers fade-in and fade-out
//version2, optimized. Create bit pattern once per number instead of every frame

#include <SPI.h>

const byte LEpin = 10;                                              //pin Latch Enabled data accepted while HI level
SPISettings settingsA(8000000, MSBFIRST, SPI_MODE2);                //SPI setup for HV5122PJ

String stringToDisplay;                                             // Content of this string will be displayed on tubes (must be 6 chars length)
String prevstringToDisplay;                                         // Content of the previous string for fading purposes
bool          newNumber;                                            //are we displaying a new number?

unsigned int SymbolArray[10] = {1,     2,     4,     8,     16,    32,    64,    128,   256,    512}; // used to translate number (0-9) to bit pattern


/*******************************************************************************************************
  Init Programm
*******************************************************************************************************/
void setup()
{

  Serial.begin(115200);
  Serial.println(F("Starting"));

  //Setup SPI communication, see https://www.arduino.cc/en/Tutorial/SPITransaction for example & documentation

  pinMode(LEpin, OUTPUT);
  SPI.begin();
  SPI.beginTransaction(settingsA);
}

/***************************************************************************************************************
  MAIN Programm
***************************************************************************************************************/
void loop() {
  static int i = 1;
  prevstringToDisplay = String("000000");
  do {
    if (i < 10) {
      stringToDisplay = String("00000") + String(i);
    } else if (i < 100) {
      stringToDisplay = String("0000") + String(i);
    } else if (i < 1000) {
      stringToDisplay = String("000") + String(i);
    } else if (i < 10000) {
      stringToDisplay = String("00") + String(i);
    } else if (i < 100000) {
      stringToDisplay = String("0") + String(i);
    }
    doIndication();
    i++;
    prevstringToDisplay = stringToDisplay;
    newNumber = true;                                        //need to refresh the bit patterns
  } while (i < 999999);
}

void doIndication()
{
  static unsigned long FPSTuner;                              //Used to autotune number of frames that fit in a minute
  static int           currentFadeCycle;                      //we want to progressively make new digit brighter so cycles need to go up. Will be calculated every frame
  static int           fadeFrameCounter;                      //keep track of when we last displayed the new frame
  static int           overallFrameCounter;                   //used to keep track of how many frames we already did in the cuurent fadecycle
  static int           nominalFPS = 12000;                     //used to hold what is an achieveable FPS with our board and code. Start point, will be autotuned

  static unsigned long pstring_var32_L = 0;                   //store the bit pattern of the prevstringToDisplay so we can reuse
  static unsigned long pstring_var32_H = 0;                   //store the bit pattern of the prevstringToDisplay so we can reuse
  static unsigned long cstring_var32_L = 0;                   //store the bit pattern of the stringToDisplay so we can reuse
  static unsigned long cstring_var32_H = 0;                   //store the bit pattern of the stringToDisplay so we can reuse

  static long          digits;                                //used in translation of stringtodisplay to bitpattern

  static unsigned long var32_L = 0;                           //Will hold 2nd 32 bits to send over
  static unsigned long var32_H = 0;                           //Will hold 1st 32 bits to send over
  static unsigned long tempVar = 0;                           //Hold temp value before setting the bits for 64 bchannel chip
  

  
  if (newNumber) {                              //setup the bitpatterns (once per new number)
    digits = stringToDisplay.toInt();
    //--Reg1 for stringToDisplay--
    tempVar = 0;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 20; // s2
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 10; //s1
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]); //m2
    digits = digits / 10;

    for (int i=1; i<=32; i++) {
     i=i+32;
     int newindex=16*(3-(ceil((float)i/4)*4-i))+ceil((float)i/4); 
     i=i-32;
     if (newindex<=32) bitWrite(cstring_var32_L, newindex-1, bitRead(tempVar, i-1));
      else bitWrite(cstring_var32_H, newindex-32-1, bitRead(tempVar, i-1));
    }

    //--Reg0 for stringToDisplay--
    tempVar = 0;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 20; // m1
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 10; //h2
    digits = digits / 10;
    tempVar |= (unsigned long)SymbolArray[digits % 10]; //h1
    digits = digits / 10;

    for (int i=1; i<=32; i++) {
     int newindex=16*(3-(ceil((float)i/4)*4-i))+ceil((float)i/4); 
     if (newindex<=32) bitWrite(cstring_var32_L, newindex-1, bitRead(tempVar, i-1));
      else bitWrite(cstring_var32_H, newindex-32-1, bitRead(tempVar, i-1));
    }

    digits = prevstringToDisplay.toInt();
    //--Reg1 for prevstringToDisplay--
    tempVar = 0;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 20; // s2
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 10; //s1
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]); //m2
    digits = digits / 10;

    for (int i=1; i<=32; i++) {
     i=i+32;
     int newindex=16*(3-(ceil((float)i/4)*4-i))+ceil((float)i/4); 
     i=i-32;
     if (newindex<=32) bitWrite(pstring_var32_L, newindex-1, bitRead(tempVar, i-1));
      else bitWrite(pstring_var32_H, newindex-32-1, bitRead(tempVar, i-1));
    }

    //--Reg0 for prevstringToDisplay--
    tempVar = 0;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 20; // m1
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 10; //h2
    digits = digits / 10;
    tempVar |= (unsigned long)SymbolArray[digits % 10]; //h1
    digits = digits / 10;

    for (int i=1; i<=32; i++) {
     int newindex=16*(3-(ceil((float)i/4)*4-i))+ceil((float)i/4); 
     if (newindex<=32) bitWrite(pstring_var32_L, newindex-1, bitRead(tempVar, i-1));
      else bitWrite(pstring_var32_H, newindex-32-1, bitRead(tempVar, i-1));
    }

    newNumber = false;          //done for now
  }


  // Initialize our variables
  fadeFrameCounter = 0;                                       //just beginning
  overallFrameCounter = 0;                                    //initialize frames done to 0, we're just beginning

  FPSTuner = millis();
  do {
    if (overallFrameCounter <= nominalFPS/2) {
      currentFadeCycle = ((float)nominalFPS / (float)overallFrameCounter);
      if (fadeFrameCounter >= currentFadeCycle) {
        var32_L = cstring_var32_L;
        var32_H = cstring_var32_H;
        fadeFrameCounter = 0;
      } else {
        var32_L = pstring_var32_L;
        var32_H = pstring_var32_H;
      }
    }  else {
      currentFadeCycle = (float)nominalFPS / ((float)nominalFPS - (float)overallFrameCounter);
      if (fadeFrameCounter >= currentFadeCycle) {
        var32_L = pstring_var32_L;
        var32_H = pstring_var32_H;
        fadeFrameCounter = 0;
      } else {
        var32_L = cstring_var32_L;
        var32_H = cstring_var32_H;
      }
    }

    digitalWrite(LEpin, LOW);
    SPI.transfer(var32_H >> 24);
    SPI.transfer(var32_H >> 16);
    SPI.transfer(var32_H >> 8);
    SPI.transfer(var32_H);

    SPI.transfer(var32_L >> 24);
    SPI.transfer(var32_L >> 16);
    SPI.transfer(var32_L >> 8);
    SPI.transfer(var32_L);
    digitalWrite(LEpin, HIGH);

    fadeFrameCounter++;                                 //up the framecounter
    overallFrameCounter++;                              //up the overall frame counter
  } while (overallFrameCounter < nominalFPS);

  //Correct nominalFPS based on actual time spent
  if ((millis()- FPSTuner) < 1000) {
    nominalFPS++;                                  //We spent less than a second, increase number of frames in a second
  } else {
    nominalFPS--;                                  //We spent more than a second, decrease number of frames in a second
  }
  Serial.println(nominalFPS);
}
