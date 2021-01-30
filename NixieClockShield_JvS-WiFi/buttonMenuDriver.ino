/* Code to handle all the button presses and consequent actions, menu-flow is as follows:
1st time modePress: TimeDisplay
        --> If long press modeButton mode --> Goto edit Time mode --> modePress moves from HH-->MM-->SS--> HH24/12--> done (back to TimeDisplay)
2nd time modePress: DateDisplay
        --> If long press modeButton --> DateFormat select (US or EU), up/down switches mode --> modePress goes into DateEdit mode DD-MM-YY or MM-DD-YY --> done back to DateDisplay
3rd time modePress: AlarmTime
        --> If long press modeButton --> Edit AlarmHH --> AlarmMM --> Alarm on/off --> back to AlarmDisplay
4th time modePress: TubeTimer (off time)
        --> If long press modeButton --> edit offHH --> offMM --> onHH --> onMM --> Tubertimer on/off (1/0)
5th time modePress: Back to TimeDisplay

If we are in 'normal' timedisplay mode and downbutton is pressed long --> turn off the leds. Short press --> start/stop rotating colors
If we are in 'normal' timedisplay mode and upbutton is pressed long --> turn on the leds and start rotating colors. 

Overall code needs to be non-blocking and timeout after no button pressed for 15 seconds (when in modeswitching mode).
*/


//Setup metadata
//--------------

ClickButton modeButton(SET_PIN, LOW, CLICKBTN_PULLUP);
ClickButton upButton(UP_PIN, LOW, CLICKBTN_PULLUP);
ClickButton downButton(DOWN_PIN, LOW, CLICKBTN_PULLUP);

//Setup the buttons code
void buttonSetup() {

  pinMode(SET_PIN,  INPUT_PULLUP);
  pinMode(UP_PIN,  INPUT_PULLUP);
  pinMode(DOWN_PIN,  INPUT_PULLUP);

 //buttons objects inits
  modeButton.debounceTime   = 20;   // Debounce timer in ms
  modeButton.multiclickTime = 30;  // Time limit for multi clicks
  modeButton.longClickTime  = 2000; // time until "held-down clicks" register

  upButton.debounceTime   = 20;   // Debounce timer in ms
  upButton.multiclickTime = 30;  // Time limit for multi clicks
  upButton.longClickTime  = 2000; // time until "held-down clicks" register

  downButton.debounceTime   = 20;   // Debounce timer in ms
  downButton.multiclickTime = 30;  // Time limit for multi clicks
  downButton.longClickTime  = 2000; // time until "held-down clicks" register

  editMode = modeChanger = false;
}

//---------------------
//Handle button actions
void handleButtons() {

  static int changeValueBy = 0;       //used for upping or dowing value while in edit mode and up/down button is pressed

  modeButton.Update();
  upButton.Update();
  downButton.Update();

  //First check if user stopped doing anything
  if ((editMode || modeChanger) && ((millis()-lastTimeButtonPressed) > NOBUTTON_TIMEOUT)) {
    editMode = modeChanger = false;
    menuPosition = TIMEINDEX;
    return;                             //back to normal
  }

  //3 basic modes: 1) timeKeeping (default), 2) modeChanger (mode was pressed) and 3) edit mode

  //---------------------EDIT MODE CODE--------------------
  if (editMode) { 
    switch (menuPosition) {
      case TIMEINDEX:
        if (editIndex > 3) {           //pos 0==HH, 1==MM, 2==SS, 3==24H/12H
          setTime(editValues[0], editValues[1], editValues[2], day(), month(), year());
          setRTCDateTime(hour(), minute(), second(), day(), month(), year() % 1000, 1);
          storedValues[T_MODE] = editValues[3];
          editMode = false;
          return;
        }
        if (editIndex != 3)
          stringToDisplay = preZero(editValues[0]) + preZero(editValues[1]) + preZero(editValues[2]);
        else
          stringToDisplay = "00" + preZero(editValues[editIndex]) + "00";
        blinkMask = editBlinkMask[editIndex];
      break;

      case DATEINDEX:
        if (editIndex > 3) {            //pos 0==D_Format, 1==DD, 2==MM, 3==YY
          storedValues[D_FORMAT] = editValues[0];
          EEPROM.write(DATEFORMAT_EEPROM_ADDR, storedValues[D_FORMAT]);
          setTime(hour(), minute(), second(), editValues[1], editValues[2], 2000 + editValues[3]);
          editMode = false;
          return;
        }

        if (editIndex == 0) {                           //DateFormat select
          if (editValues[editIndex] == EU_DF)
            stringToDisplay = "311299";
          else
            stringToDisplay = "123199";
          blinkMask = editBlinkMask[editIndex];
        } else {
          if (editValues[0] == EU_DF){
            stringToDisplay = preZero(editValues[1]) + preZero(editValues[2]) + preZero(editValues[3]);
          } else {
            stringToDisplay = preZero(editValues[2]) + preZero(editValues[1]) + preZero(editValues[3]);
          }
          blinkMask = editBlinkMask[editIndex];
        }
      break;

      case ALARMINDEX:
          if (editIndex > 2) {          //pos 0=Alarm HH, 1=Alarm MM, 2=Alarm on/off ( we are not using alarm seconds --> set to 0)
            storedValues[A_HOUR]    = editValues[0];
            storedValues[A_MINUTE]  = editValues[1];
            storedValues[A_SECOND]  = 0;
            storedValues[A_ON]      = editValues[2];
            EEPROM.write(ALARMTIME_EEPROM_ADDR, storedValues[A_HOUR]);
            EEPROM.write(ALARMTIME_EEPROM_ADDR + 1, storedValues[A_MINUTE]);
            EEPROM.write(ALARMTIME_EEPROM_ADDR + 2, storedValues[A_SECOND]);
            EEPROM.write(ALARMARMED_EEPROM_ADDR, storedValues[A_ON]);
            editMode = false;
            return;
          }

          if (editIndex != 2) {
            stringToDisplay = preZero(editValues[0]) + preZero(editValues[1]) + "00";
          } else {
            stringToDisplay = "00" + preZero(editValues[2]) + "00";
          }
          blinkMask = editBlinkMask[editIndex];
      break;

      case TTIMERINDEX:
        if (editIndex > 4) {          //pos 0=Timer off HH, 1=Timer off MM, 2=Timer on HH, 3=Timer on MM, 4=Tubertimer on or off
            storedValues[TOFF_HOUR]   = editValues[0];
            storedValues[TOFF_MINUTE] = editValues[1];
            storedValues[TON_HOUR]    = editValues[2];
            storedValues[TON_MINUTE]  = editValues[3];
            storedValues[TT_ON]       = editValues[4];
            Serial.println("about to write: " + String(storedValues[TT_ON]));

            EEPROM.write(TT_OFF_HH_EEPROM_ADDR, storedValues[TOFF_HOUR]); 
            EEPROM.write(TT_OFF_MM_EEPROM_ADDR, storedValues[TOFF_MINUTE]); 
            EEPROM.write(TT_ON_HH_EEPROM_ADDR, storedValues[TON_HOUR]); 
            EEPROM.write(TT_ON_MM_EEPROM_ADDR, storedValues[TON_MINUTE]); 
            EEPROM.write(TT_ENABLE_EEPROM_ADDR, storedValues[TT_ON]); 

            Serial.println("Read from EEPROM2: " + String(EEPROM.read(TT_ENABLE_EEPROM_ADDR)));
            editMode = false;
            return;
          }

        if (editIndex == 0 || editIndex == 1) {
          stringToDisplay = preZero(editValues[0]) + preZero(editValues[1]) + "00";   //Tubetimer off time
        } else if (editIndex == 2 || editIndex == 3) {
          stringToDisplay = preZero(editValues[2]) + preZero(editValues[3]) + "00";   //Tubetimer on time
        } else if (editIndex == 4) {
          stringToDisplay = "00" + preZero(editValues[4]) + "00";                     //Tubetimer on(1) or off(0)
        }
        blinkMask = editBlinkMask[editIndex];
      break;
    }                                   // end switch (menuposition)

    if (modeButton.clicks > 0) {        //single mode-click registered while in edit mode
      tone1.play(1000, 100);
      lastTimeButtonPressed = millis();
      editIndex++;
      return;                          //one button at the time
    }

    changeValueBy = 0;
    if (upButton.clicks > 0) {
      //need to increase something
      lastTimeButtonPressed = millis();
      tone1.play(1000, 100);
      changeValueBy = 1;
    }

    if (downButton.clicks > 0) {
      //need to decrease something
      lastTimeButtonPressed = millis();
      tone1.play(1000, 100);
      changeValueBy = -1;
    }

    if (changeValueBy != 0) {
      Serial.print("menuPosition: ");
      Serial.println(menuPosition);
      Serial.print("editIndex: ");
      Serial.println(editIndex);
      switch (menuPosition) {
        case TIMEINDEX:
          if (editIndex == 0) changeValue(&editValues[0], changeValueBy, 0, 23);
          if (editIndex == 1) changeValue(&editValues[1], changeValueBy, 0, 59);
          if (editIndex == 2) changeValue(&editValues[2], changeValueBy, 0, 59);
        break;
        case DATEINDEX:
          if (editIndex == 0) {
            if (editValues[0] == EU_DF) editValues[0] = US_DF;
              else editValues[0] = EU_DF;
          }
          if (editIndex == 1) changeValue(&editValues[1], changeValueBy, 0, 31);
          if (editIndex == 2) changeValue(&editValues[2], changeValueBy, 0, 12);
          if (editIndex == 3) changeValue(&editValues[3], changeValueBy, 0, 99);
        break;
        case ALARMINDEX:
          if (editIndex == 0) changeValue(&editValues[0], changeValueBy, 0, 23);
          if (editIndex == 1) changeValue(&editValues[1], changeValueBy, 0, 59);
          if (editIndex == 2) {
            if (editValues[2] == 0) editValues[2] = 1;
              else editValues[2] = 0;
          }
        break;
        case TTIMERINDEX:
          if (editIndex == 0 or editIndex == 2) changeValue(&editValues[editIndex], changeValueBy, 0, 23);
          if (editIndex == 1 or editIndex == 3) changeValue(&editValues[editIndex], changeValueBy, 0, 59);
          if (editIndex == 4) {
            if (editValues[4] == 0) {
              editValues[4] = 1;
              LD = true;
            } else {
              editValues[4] = 0;
              LD = false;
            }
          }
        break;
      }
    }
    return;                            //All we want to do in edit mode!
  }                                    // end if editmode
  //-----------------END EDIT MODE CODE------------------------
  //-----------------------------------------------------------
  //-----------------------------------------------------------
  //-----------------START MODE CHANGE CODE--------------------
  if (modeChanger) {
    switch (menuPosition) {
      case TIMEINDEX:
        //Time is already frozen, nothing to do
      break;
      case DATEINDEX:
        if (storedValues[D_FORMAT] == EU_DF) {
          stringToDisplay = preZero(day()) + preZero(month()) + preZero(year()-2000);
        } else {
          stringToDisplay = preZero(month()) + preZero(day()) + preZero(year()-2000);
        }
      break;
      case ALARMINDEX:
        stringToDisplay = preZero(storedValues[A_HOUR]) + preZero(storedValues[A_MINUTE]) + "00";
      break;
      case TTIMERINDEX:
        stringToDisplay = preZero(storedValues[TOFF_HOUR]) + preZero(storedValues[TOFF_MINUTE]) + "00";
      break;
    }

    if (modeButton.clicks > 0) {       //short press modebutton --> increase menuposition
      TubesOn = true;
      tone1.play(1000, 100);
      menuPosition++;
      Serial.println(menuPosition);
      lastTimeButtonPressed = millis();
      if (menuPosition > MAXMENUPOS) {
        menuPosition = TIMEINDEX;       //back to showing the time
        modeChanger = false;           //and also back to default displaying the time
        return;
      }
    }

    if (modeButton.clicks < 0) {       //entering edit mode
      TubesOn = true;
      tone1.play(1000, 100);
      lastTimeButtonPressed = millis();
      editMode = true;
      editIndex = 0;                   //just beginning

      switch (menuPosition) {           //setup start values for the edit
        case TIMEINDEX:
          editValues[0]    = hour();
          editBlinkMask[0] = B00000011;
          editValues[1]    = minute();
          editBlinkMask[1] = B00001100;
          editValues[2]    = second();
          editBlinkMask[2] = B00110000;
          editValues[3]    = storedValues[T_MODE];
          editBlinkMask[3] = B00001100;
        break;
        case DATEINDEX:
          editValues[0]    = storedValues[D_FORMAT];
          editBlinkMask[0] = B00111111;
          editValues[1]    = day();
          editValues[2]    = month();
          if (editValues[0] == EU_DF) {
            editBlinkMask[1] = B00000011;
            editBlinkMask[2] = B00001100;
          } else {
            editBlinkMask[1] = B00001100;
            editBlinkMask[2] = B00000011;
          }
          editValues[3]    = year() - 2000;
          editBlinkMask[3] = B00110000;
        break;
        case ALARMINDEX:
          editValues[0]    = storedValues[A_HOUR];
          editBlinkMask[0] = B00000011;
          editValues[1]    = storedValues[A_MINUTE];
          editBlinkMask[1] = B00001100;
          editValues[2]    = storedValues[A_ON];
          editBlinkMask[2] = B00001100;
        break;
        case TTIMERINDEX:
          editValues[0]    = storedValues[TOFF_HOUR];
          editBlinkMask[0] = B00000011;
          editValues[1]    = storedValues[TOFF_MINUTE];
          editBlinkMask[1] = B00001100;
          editValues[2]    = storedValues[TON_HOUR];
          editBlinkMask[2] = B00000011;
          editValues[3]    = storedValues[TON_MINUTE];
          editBlinkMask[3] = B00001100;
          editValues[4]    = storedValues[TT_ON];
          editBlinkMask[4] = B00001100;
          if (editValues[4] == 1) LD=true;
            else LD=false;
        break;
      }
      return;                          //one button at the time
    }
    return;                            //We don't want anything else registered while in mode change
  }
  //-----------------END MODE CHANGE CODE----------------------
  //-----------------------------------------------------------
  //---------START SHORT PRESS CODE, NOT IN SPECIAL MODE-------

  if (modeButton.clicks > 0) {
    Serial.println("Mode button registered");
    lastTimeButtonPressed = millis();
    modeChanger = true;
    tone1.play(1000, 100);
    menuPosition = TIMEINDEX;
    stringToDisplay = preZero(hour()) + preZero(minute()) + preZero(second());  //freeze time displayed on first mode press
    return;
  }

  if (downButton.clicks < 0 && RGBLedsOn) {       //down long press --> turn off the leds
    TubesOn = true;
    tone1.play(1000, 100);
    RGBLedsOn = false;
    EEPROM.write(RGB_EEPROM_ADDR, 0);
    return;
  }

  if (downButton.clicks > 0 && RGBLedsOn) {       //down short press --> toggle LedsLock
    TubesOn = true;
    tone1.play(1000, 100);
    LEDsLock = !LEDsLock;
    if (LEDsLock) {
      EEPROM.write(LEDLOCK_EEPROM_ADDR, 1);
      EEPROM.write(LEDREDVAL_EEPROM_ADDR, RedLight);
      EEPROM.write(LEDGREENVAL_EEPROM_ADDR, GreenLight);
      EEPROM.write(LEDBLUEVAL_EEPROM_ADDR, BlueLight);
    }
    return;
  }

  if (upButton.clicks < 0 && !RGBLedsOn) {         //Upbutton long press --> turn on the leds
    TubesOn = true;
    tone1.play(1000, 100);
    RGBLedsOn = true;
    EEPROM.write(RGB_EEPROM_ADDR, 1);
    return;
  }
  //---------END SHORT PRESS CODE, NOT IN SPECIAL MODE-------
  //---------------------------------------------------------
} //---------END BUTTONHANDLER CODE

//Helper function changevalue, to handle up and down increase/decrease

void changeValue(int *input, int increment, int min, int max) {
  *input = *input + increment;
  if (*input > max) *input = min;
  if (*input < min) *input = max;
  Serial.print("changedValue = ");
  Serial.println(*input);
}
