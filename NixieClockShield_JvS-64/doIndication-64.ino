//driver for NCM107+NCT318+NCT818, NCS312, NCS314 HW2.x (registers HV5122)
//driver version 1.2
//1 on register's output will turn on a digit


//v1.2 SPI setup moved to driver's file
//v1.1 Mixed up on/off for dots

#include "doIndication-64.h"

#define UpperDotsMask 0x80000000
#define LowerDotsMask 0x40000000

SPISettings settingsA(1000000, MSBFIRST, SPI_MODE3);                //SPI setup for HV5122PJ

void SPISetup()
{
  SPI.begin();
  SPI.beginTransaction(settingsA);
}

void doIndication()
{

  static unsigned long lastTimeInterval1Started;
  static unsigned long lastTimeInterval2Started;
  static unsigned long nominalFPS=1800;                       //start conservative, the actual FPS will autotune and right itself in a short while.

  static unsigned long FPSTuner;                              //Used to autotune number of frames that fit in a minute
  static int           currentFadeCycle;                      //we want to progressively make new digit brighter so cycles need to go up. Will be calculated every frame
  static int           fadeFrameCounter;                      //keep track of when we last displayed the new frame
  static int           overallFrameCounter;                   //used to keep track of how many frames we already did in the cuurent fadecycle
  static int           lastSecond = second();
  static bool          fadeInProgress = false;                //keep track wether we have a fade transaction going, this to make sure we don't start the next one before this one is finished
  static bool          dotsOn = false;                        //for normal blinking (not in edit mode), turn on or off once per second (so 2 second rithm)

  static unsigned long pstring_var32_L = 0;                   //store the bit pattern of the prevstringToDisplay so we can reuse
  static unsigned long pstring_var32_H = 0;                   //store the bit pattern of the prevstringToDisplay so we can reuse
  static unsigned long cstring_var32_L = 0;                   //store the bit pattern of the stringToDisplay so we can reuse
  static unsigned long cstring_var32_H = 0;                   //store the bit pattern of the stringToDisplay so we can reuse

  long digits;                                                //used in translation of stringtodisplay to bitpattern
  unsigned long var32_L = 0;                                  //Will hold 2nd 32 bits to send over
  unsigned long var32_H = 0;                                  //Will hold 1st 32 bits to send over
  unsigned long tempVar = 0;                                  //Will hold temporary bitpattern, for building only

  if (newNumber) {                              //initialize fade for next timeposition
    if (fadeInProgress) nominalFPS--;           //we used more frames than fit in 1 second, tune down a bit.
    dotsOn = !dotsOn;                           //turn the dots on or off
    fadeInProgress = true;
    FPSTuner = millis();                        //keep track of how long the fade takes, for autotuning. Works tw. nominalFPS (in.h file)
    fadeFrameCounter = 0;
    overallFrameCounter = 0;
    lastSecond=second();

    digits = stringToDisplay.toInt();
    //--Reg1 for stringToDisplay--
    tempVar = 0;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 20; // s2
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]) << 10; //s1
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]); //m2
    digits = digits / 10;

    if (dotsOn) {
      tempVar|=LowerDotsMask;
      tempVar|=UpperDotsMask;
    } else {
      tempVar&=~LowerDotsMask;
      tempVar&=~UpperDotsMask;
    }

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

    if (dotsOn) {
      tempVar|=LowerDotsMask;
      tempVar|=UpperDotsMask;
    } else {
      tempVar&=~LowerDotsMask;
      tempVar&=~UpperDotsMask;
    }

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

    if (dotsOn) {
      tempVar|=LowerDotsMask;
      tempVar|=UpperDotsMask;
    } else {
      tempVar&=~LowerDotsMask;
      tempVar&=~UpperDotsMask;
    }

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

    if (dotsOn) {
      tempVar|=LowerDotsMask;
      tempVar|=UpperDotsMask;
    } else {
      tempVar&=~LowerDotsMask;
      tempVar&=~UpperDotsMask;
    }

    for (int i=1; i<=32; i++) {
     int newindex=16*(3-(ceil((float)i/4)*4-i))+ceil((float)i/4); 
     if (newindex<=32) bitWrite(pstring_var32_L, newindex-1, bitRead(tempVar, i-1));
      else bitWrite(pstring_var32_H, newindex-32-1, bitRead(tempVar, i-1));
    }

    newNumber = false;
  }

  if ((menuPosition == TimeIndex) & !transactionInProgress) {  //we are in timedisplay mode
    if (overallFrameCounter <= nominalFPS / 2) {
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

    fadeFrameCounter++;                                 //up the framecounter within current fadecycle
    overallFrameCounter++;                              //up the overall frame counter
  } 

  if (menuPosition != TimeIndex || transactionInProgress) {                                              //we are not in time displaymode
    digits = stringToDisplay.toInt();
    tempVar = 0;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(5)) << 20; // s2
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(4)) << 10; //s1
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(3)); //m2
    digits = digits / 10;

    if (LD) tempVar|=LowerDotsMask;
      else  tempVar&=~LowerDotsMask;
    if (UD) tempVar|=UpperDotsMask;
      else tempVar&=~UpperDotsMask;

    for (int i=1; i<=32; i++) {
     i=i+32;
     int newindex=16*(3-(ceil((float)i/4)*4-i))+ceil((float)i/4); 
     i=i-32;
     if (newindex<=32) bitWrite(var32_L, newindex-1, bitRead(tempVar, i-1));
      else bitWrite(var32_H, newindex-32-1, bitRead(tempVar, i-1));
    }

    tempVar = 0;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(2)) << 20; // m1
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(1)) << 10; //h2
    digits = digits / 10;
    tempVar |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(0)); //h1
    digits = digits / 10;

    if (LD) tempVar|=LowerDotsMask;
      else  tempVar&=~LowerDotsMask;
    if (UD) tempVar|=UpperDotsMask;
      else tempVar&=~UpperDotsMask;

    for (int i=1; i<=32; i++) {
     int newindex=16*(3-(ceil((float)i/4)*4-i))+ceil((float)i/4); 
     if (newindex<=32) bitWrite(var32_L, newindex-1, bitRead(tempVar, i-1));
      else bitWrite(var32_H, newindex-32-1, bitRead(tempVar, i-1));
    }
  }

  if (overallFrameCounter >= nominalFPS) {                //the fade has finished, clean up
    fadeInProgress = false;
    
    if ((millis() - FPSTuner) < 1000) {                  //Correct nominalFPS based on actual time spent
      nominalFPS++;                                      //We spent less than a second, increase number of frames in a second
    }
    Serial.println(nominalFPS);
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
  //-------------------------------------------------------------------------
}

word doEditBlink(int pos)
{
  /*
    if (!BlinkUp) return 0;
    if (!BlinkDown) return 0;
  */

  if (!BlinkUp) return 0xFFFF;
  if (!BlinkDown) return 0xFFFF;
  //if (pos==5) return 0xFFFF; //need to be deleted for testing purpose only!
  int lowBit = blinkMask >> pos;
  lowBit = lowBit & B00000001;

  static unsigned long lastTimeEditBlink = millis();
  static bool blinkState = false;
  word mask = 0xFFFF;
  static int tmp = 0; //blinkMask;
  if ((millis() - lastTimeEditBlink) > 300)
  {
    lastTimeEditBlink = millis();
    blinkState = !blinkState;
    if (blinkState) tmp = 0;
    else tmp = blinkMask;
  }
  if (((dotPattern & ~tmp) >> 6) & 1 == 1) LD = true; //digitalWrite(pinLowerDots, HIGH);
    else LD = false; //digitalWrite(pinLowerDots, LOW);
  if (((dotPattern & ~tmp) >> 7) & 1 == 1) UD = true; //digitalWrite(pinUpperDots, HIGH);
    else UD = false; //digitalWrite(pinUpperDots, LOW);

  if ((blinkState == true) && (lowBit == 1)) mask = 0x3C00; //mask=B11111111;
  //Serial.print("doeditblinkMask=");
  //Serial.println(mask, BIN);
  return mask;
}

word blankDigit(int pos)
{
  int lowBit = blankMask >> pos;
  lowBit = lowBit & B00000001;
  word mask = 0;
  if (lowBit == 1) mask = 0xFFFF;
  return mask;
}
