DumbUPS - A really stupid UPS daemon for really stupid UPSs
=====================

This is a go project and firmware for interfacing with UPSs that don't have modern interfaces. 
Why? they are perfectly fine and often available second hand :)

Currently this is only tested against a APC BackUPS Pro 1400 via the example arduino-based interface device,
but the existing interface should work for most dumb ups's.

Go Program
=======
The go part of this project (this directory) has two commands:
 
 * `monitor` 
   a continuously running process that reads the  
serial port that's connected to the UPS interface microcontroller (see Protocol below). 

The [config](dumbups.conf.sample) file requires the serial port. 
The monitor daemon will use the optional "shutdown" config option to specify if it will issue a shutdown upon power loss (the default) or low battery

The Protocol
====

The protocol is very simple, the UPS interface outputs lines of text representing the state:
```
BATT:<bool>,BATTLOW:<bool>,MAINSDWN:<bool>,MAINSUP:<bool>\n
```

where <bool> is either "Yes" or "No" if that line is high.

UPS Interfaces
====

 * [APC](Arduino/APC_Interface/Readme.md)

   This implementation example uses an arduino (specifically a Leostick) but should be simple enough to serve as an example for other interfaces
