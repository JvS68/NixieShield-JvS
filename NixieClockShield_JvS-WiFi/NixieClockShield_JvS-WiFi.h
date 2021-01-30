//Include file for NixieClockShield w. WiFi

#include <SPI.h>
#include <Wire.h>
#include <ClickButton.h>
#include <TimeLib.h>
#include <Timezone.h> 
#include <Tone.h>
#include <EEPROM.h>
#include <OneWire.h>

#include "TubeDriver.h"

#define tubes6

#define RedLedPin					9 //MCU WDM output for red LEDs 9-g
#define GreenLedPin					6 //MCU WDM output for green LEDs 6-b
#define BlueLedPin					3 //MCU WDM output for blue LEDs 3-r
#define pinBuzzer					2
#define pinUpperDots				12 //HIGH value light a dots
#define pinLowerDots				8 //HIGH value light a dots
#define pinTemp						7
#define SET_PIN                		PIN_A0
#define DOWN_PIN               		PIN_A1
#define UP_PIN                 		PIN_A2

#define DS1307_ADDRESS 				0x68

#define timeModePeriod 60000
#define dateModePeriod 5000

// EEPROM Addresses
#define RGB_EEPROM_ADDR         0
#define HOURFORMAT_EEPROM_ADDR  1
#define ALARMTIME_EEPROM_ADDR   2 //2, 3, 4
#define ALARMARMED_EEPROM_ADDR  6
#define LEDLOCK_EEPROM_ADDR     7
#define LEDREDVAL_EEPROM_ADDR   8
#define LEDGREENVAL_EEPROM_ADDR 9
#define LEDBLUEVAL_EEPROM_ADDR  10
#define DEGFORMAT_EEPROM_ADDR   11
#define DATEFORMAT_EEPROM_ADDR  12
#define TT_OFF_HH_EEPROM_ADDR   13
#define TT_OFF_MM_EEPROM_ADDR   14
#define TT_ON_HH_EEPROM_ADDR    15
#define TT_ON_MM_EEPROM_ADDR    16
#define TT_ENABLE_EEPROM_ADDR   17

#define NTP_BUFFER_LENGTH   100
#define NTP_SYNC_INTERVAL   1000 * 5

#define T_HOUR 		         	0
#define T_MINUTE 		       	1
#define T_SECOND 		       	2
#define T_MODE         			3
#define D_FORMAT         		4 
#define D_DAY            		5
#define D_MONTH          		6
#define D_YEAR           		7
#define A_HOUR          		8
#define A_MINUTE        		9
#define A_SECOND        		10
#define A_ON                 	11
#define TOFF_HOUR   			12
#define TOFF_MINUTE    			13
#define TON_HOUR    			14
#define TON_MINUTE     			15
#define TT_ON         			16

int storedValues[TT_ON];

#define US_DF		 				1
#define EU_DF		 				0
#define CELSIUS 					0
#define FAHRENHEIT 					1

#define TIMEINDEX             	  	0
#define DATEINDEX               	1
#define ALARMINDEX              	2
#define TTIMERINDEX          		3
#define MAXMENUPOS					TTIMERINDEX


//FOR BUTTONMENUDRIVER


#define NOBUTTON_TIMEOUT			15000		//if no button is pressed for 15 seconds, clean up & go to default state

bool 			editMode;						//we are editing something (time, date, alarm or tubetimer)
bool 			modeChanger;					//user pressed mode, we are not in default timekeeping mode
unsigned long	lastTimeButtonPressed = 0;		//keep track of when we changed into
int 			menuPosition = 0;				//keep track of which menu position we are in, this is used in the mainloop to determine what to display
int 			editIndex = 0;					//Keep track of which number of the menu position we are editing.
int             editValues[5];					//store the values for the mode being edited
int 			editBlinkMask[5];				//store the current blink masks for current edit mode (ie time, date, alarm etc)
byte            blinkMask = B00000000;


//FOR NTP TIME SYNC
char ntp_buffer[NTP_BUFFER_LENGTH];
byte ntp_position = 0;
unsigned long prev_ntp_sync = 0;
bool ntp_sync_pending = true;

struct sNTP_DATE_TIME
{
  byte ntp_seconds;
  byte ntp_minutes;
  byte ntp_hours;
  byte ntp_day;
  byte ntp_month;
  byte ntp_year;
};

sNTP_DATE_TIME ntp_date_time;

TimeChangeRule *tcr;        // pointer to the time change rule, use to get the TZ abbrev

// US Pacific Time Zone (Las Vegas, Los Angeles)
// for more examples on your timezone: https://github.com/JChristensen/Timezone
TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);
Timezone myTZ(usPDT, usPST);