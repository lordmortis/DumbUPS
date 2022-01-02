DumbUPS - A really stupid UPS daemon for really stupid UPSs
=====================

This is a go project and firmware for interfacing with UPSs that don't have modern interfaces. 
Why? they are perfectly fine and often available second hand :)

However, this is only tested against a APC BackUPS Pro 1400 via the example arduino-based interface device,
but the existing interface should work for most dumb UPS's.

Go Program
=======
The go part of this project (this directory) has two commands:
 
 * `monitor` 
   a continuously running process that reads the  
   serial port that's connected to the UPS interface microcontroller (see Protocol below). 
   It will also emit a "pong" command every 10 seconds to reset the watchdog timer
 * `shutdown`
   send the "shut" command to the micro to turn off the power.

The [config](dumbups.conf.sample) file requires the serial port. 
The monitor daemon will use the optional "shutdown" config option to specify if it will issue a shutdown upon power loss (the default) or low battery

The Protocol
====

The protocol is very simple, the UPS interface outputs lines of text representing the state:
```
STATUS-BATT:<bool>,BATTLOW:<bool>,MAINSDWN:<bool>,MAINSUP:<bool>\n
```

where <bool> is either "Yes" or "No" if that line is high.

There is a 30-second watchdog timer, where if no command is received for this period and the UPS has no power, it will turn off.

Commands:
 * `shut` - start the power off sequence if the mains power is off
 * `noshut` - cancel a power off sequence
 * `ping` - a no-op that resets the watchdog timer

UPS Interfaces
====

 * [APC - Arduino](Arduino/APC_Interface/Readme.md)
   * This implementation example uses an arduino (specifically a [LeoStick](https://www.freetronics.com.au/products/leostick)) but should be simple enough to serve as an example for other interfaces
 * [APC - attiny2312](attiny2313/Readme.md)
   * This implementation is for an [attiny2313](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2543-AVR-ATtiny2313_Datasheet.pdf) - which is a much simpler micro. Also includes Schematics and Gerbers for a PCB!

Linux Service
====
There is an example systemd file for running this as a linux service [here](systemd/dumbups.service)
