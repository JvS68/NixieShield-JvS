

#include <ClickButton.h>

#define pinMode						A0
#define pinDown						A1
#define pinUp						A2

#define US_DF		 				1
#define EU_DF		 				0
#define CELSIUS 					0
#define FAHRENHEIT 					1

#define TIMEINDEX             	  	0
#define DATEINDEX               	1
#define ALARMINDEX              	2
#define TTIMERINDEX          		3
#define MAXMENUPOS					TTIMERINDEX




#define NOBUTTON_TIMEOUT			15000		//if no button is pressed for 15 seconds, clean up & go to default state

bool 			editMode = false;				//we are editing something (time, date, alarm or tubetimer)
bool 			modeChanger = false;			//user pressed mode, we are not in default timekeeping mode
unsigned long	lastTimeButtonPressed = 0;		//keep track of when we changed into
int 			menuPosition = 0;				//keep track of which menu position we are in, this is used in the mainloop to determine what to display
int 			editIndex = 0;					//Keep track of which number of the menu position we are editing.
int             editValues[5];					//store the values for the mode being edited
int 			editBlinkMask[5];				//store the current blink masks for current edit mode (ie time, date, alarm etc)
byte            blinkMask = B00000000;