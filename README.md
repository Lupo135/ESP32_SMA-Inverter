# ESP32_SMA-Inverter
Arduino Project to read SMA Inverter data via ESP32 bluetooth.
It ist tested with my SMA SMC6000TL with a plugin SMA bluetooth module.
Please let me know when you have tested the software on other SMA Inverters.

The starting point for this project was the code posted by "SBFspot" and "ESP32_to_SMA" on github.
Many thanks for the work on these projects!

SETUP:

Modify SmaBTAddress[] and SmaInvPass[] array in ESP32_SMA-Inverter.ino file according your system.
The SmaInvPass[] has always 12 character with 0x00 on the unused trailing character.
If SMA_WEBSERVER is usend, modify ssid and password too.


NOTES:


TODO:



KNOWN BUGS:

