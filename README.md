# NixieShield-JvS
Arduino Mega code for Nixieshield Clock w. 6 tubes

This is my personal adaptation of the Gra&Afch (https://gra-afch.com/) Nixieclock firmware. This works on an Arduino Mega with built in Wifi (ESP8266). It also works with 8266 shields but you need to change some of the code, most notably Serial3 --> Serial1 and speed from 115200 to 9600.

Most notable changes:
- stripped out Infrared remote control support as I will never use that
- stripped out GPS time-sync as also don't need that
- added Wifi support with NTP sync
- created fading digits when displaying the time
- changed the anti-poisoning to be more like a real slotmachine
- rewrote all the button press code to handle setting of time, date, alarm etc. (still a bit of WIP)

Credits:
- Gra&Afch for the original code
- buildxyx --> buildxyz-git/nixiclock_ncs314 for the NTP sync & Wifi code and the option to automatically switch off the clock at night

Button handling now is:
- mode button short presses cycles Time --> Date --> Alarm --> Clock on/off
- mode button long press enters edit mode for the previously mentioned modes
- when in 'normal' timekeeping mode a long press on 'down' shuts off the LED's, long press on 'up' button turns on the LEDS. When LED's are on short pressing the down button turns color rotation on and off.

Notes:
- Clock is set to CET and CEST standard, uses the timezone library. Easy to adapt to your own timezone, see .h file
- Because of the fading code the clock tunes itself when turned on for the 1st minute or 2, this to get the fades exactly in sync with the time. This can cause the seconds to jump a few times in the first few minutes. After that this is gone.

