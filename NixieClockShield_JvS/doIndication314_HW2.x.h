#ifndef __DOINDICATION_H
#define __DOINDICATION_H
//#include <stdint.h>
#include <Arduino.h>
unsigned int SymbolArray[10]={1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

unsigned long nominalFPS=850;  //start conservative, the actual FPS will autotune and right itself in a short while.
const byte LEpin=10;

#endif
