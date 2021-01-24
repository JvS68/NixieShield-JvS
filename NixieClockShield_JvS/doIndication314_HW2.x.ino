//driver for NCM107+NCT318+NCT818, NCS312, NCS314 HW2.x (registers HV5122)
//driver version 1.2
//1 on register's output will turn on a digit


//v1.2 SPI setup moved to driver's file
//v1.1 Mixed up on/off for dots

#include "doIndication314_HW2.x.h"

#define UpperDotsMask 0x80000000
#define LowerDotsMask 0x40000000

SPISettings settingsA(8000000, MSBFIRST, SPI_MODE2);                //SPI setup for HV5122PJ

void SPISetup()
{
  SPI.begin();
  SPI.beginTransaction(settingsA);
}

void doIndication()
{

  static unsigned long lastTimeInterval1Started;
  static unsigned long lastTimeInterval2Started;

  static unsigned long FPSTuner;                              //Used to autotune number of frames that fit in a minute
  static int           currentFadeCycle;                      //we want to progressively make new digit brighter so cycles need to go up. Will be calculated every frame
  static int           fadeFrameCounter;                      //keep track of when we last displayed the new frame
  static int           overallFrameCounter;                   //used to keep track of how many frames we already did in the cuurent fadecycle
  static int           lastSecond = second();
  static bool          fadeInProgress = false;                //keep track wether we have a fade transaction going, this to make sure we don't start the next one before this one is finished
  static bool          dotsOn = false;                        //for normal blinking (not in edit mode), turn on or off once per second (so 2 second rithm)
  long digits;                                                //used in translation of stringtodisplay to bitpattern
  unsigned long var32_L = 0;                                  //Will hold 2nd 32 bits to send over
  unsigned long var32_H = 0;                                  //Will hold 1st 32 bits to send over

  //if ((micros() - lastTimeInterval1Started) < fpsLimit) return;

  digits = stringToDisplay.toInt();

  if (lastSecond != second()) {                 //initialize fade for next timeposition
    if (fadeInProgress) nominalFPS--;           //we used more frames than fit in 1 second, tune down a bit.
    dotsOn = !dotsOn;                           //turn the dots on or off
    fadeInProgress = true;
    FPSTuner = millis();                        //keep track of how long the fade takes, for autotuning. Works tw. nominalFPS (in.h file)
    fadeFrameCounter = 0;
    overallFrameCounter = 0;
    lastSecond=second();
    Serial.println("Start fade transaction");
  }

  if ((menuPosition == TimeIndex) & !transactionInProgress & fadeInProgress) {
    if (overallFrameCounter <= nominalFPS / 2) {
      currentFadeCycle = ((float)nominalFPS / (float)overallFrameCounter);
      if (fadeFrameCounter >= currentFadeCycle) {
        digits = stringToDisplay.toInt();
        fadeFrameCounter = 0;
      } else {
        digits = prevstringToDisplay.toInt();
      }
    }  else {
      currentFadeCycle = (float)nominalFPS / ((float)nominalFPS - (float)overallFrameCounter);
      if (fadeFrameCounter >= currentFadeCycle) {
        digits = prevstringToDisplay.toInt();
        fadeFrameCounter = 0;
      } else {
        digits = stringToDisplay.toInt();
      }
    }
    fadeFrameCounter++;                                 //up the framecounter
    overallFrameCounter++;                              //up the overall frame counter
  }

  if (overallFrameCounter >= nominalFPS) {              //the fade has finished, clean up
    fadeInProgress = false;
    
    //Correct nominalFPS based on actual time spent
    if ((millis() - FPSTuner) < 1000) {
      nominalFPS++;                                  //We spent less than a second, increase number of frames in a second
    }
    Serial.println(nominalFPS);
  }

  //--Reg1--
  var32_H = 0;
  var32_H |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(5)) << 20; // s2
  digits = digits / 10;
  var32_H |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(4)) << 10; //s1
  digits = digits / 10;
  var32_H |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(3)); //m2
  digits = digits / 10;

  if (menuPosition == TimeIndex) {                    //we are in normal timekeeping mode, blink dots with the seconds
    if (dotsOn & !transactionInProgress) {
      var32_H|=LowerDotsMask;
      var32_H|=UpperDotsMask;
    } else {
      var32_H&=~LowerDotsMask;
      var32_H&=~UpperDotsMask;
    }
  } else {                                            //we are in edit mode, dot control is with the edit process
    if (LD) var32_H|=LowerDotsMask;
      else  var32_H&=~LowerDotsMask;
    if (UD) var32_H|=UpperDotsMask;
    else var32_H&=~UpperDotsMask;
  }

  //--Reg0--
  var32_L = 0;
  var32_L |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(2)) << 20; // m1
  digits = digits / 10;
  var32_L |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(1)) << 10; //h2
  digits = digits / 10;
  var32_L |= (unsigned long)(SymbolArray[digits % 10]&doEditBlink(0)); //h1
  digits = digits / 10;


  if (menuPosition == TimeIndex) {                      //we are in normal timekeeping mode, blink dots with the seconds
    if (dotsOn & !transactionInProgress) {
      var32_L|=LowerDotsMask;
      var32_L|=UpperDotsMask;
    } else {
      var32_L&=~LowerDotsMask;
      var32_L&=~UpperDotsMask;
    }
  } else {                                              //we are in edit mode, dot control is with the edit process
    if (LD) var32_L|=LowerDotsMask;
      else  var32_L&=~LowerDotsMask;
    if (UD) var32_L|=UpperDotsMask;
      else var32_L&=~UpperDotsMask;
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
