# NixieShield-JvS
Arduino Mega code for Nixieshield Clock w. 6 tubes

This is my personal adaptation of the Gra&Afch NIxieclock firmware. This works on an Arduino Mega with built in Wifi (ESP8266). It also works with 8266 shields but you need to change some of the code, most notably Serial3 --> Serial 1 and speed from 115200 to 9600.

Most notable changes:
- stripped out Infrared remote control support as I will never use that
- stripped ou GPS time-sync as also don't need that
- added Wifi support with NTP sync
- created fading digits when displaying the time
- changed the anti-poisoning to be more like a real slotmachine
- rewrote all the button press code to handle setting of time, date, alarm etc. (still a bit of WIP)

Credits:
- Gra&Afch for the original code
- buildxyx --> buildxyz-git/nixiclock_ncs314 for the NTP sync & Wifi code and the option to automatically switch off the clock at night

