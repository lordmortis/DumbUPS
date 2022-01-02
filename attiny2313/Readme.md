DumbUPS - Firmware for APC Dumb ups interface (attiny version)
=====================

This repo contains a platformio project to provide a DumbUPS serial interface for a APC UPS that uses the dumb interface.

This uses [Attiny2313V-10PU](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2543-AVR-ATtiny2313_Datasheet.pdf) and a [FTDI MM232R USB dev board](https://ftdichip.com/products/mm232r/) instead of a Leostick. 
I think this is a much smaller physical layout, with cheaper components and no surface mount, so this is easy to make.

Included is the [Kicad Project](kicad)

 * [Schematic](Schematic.pdf)
 * [PCB Layout](PCB.pdf)
 
Pinouts
=

| Signal        | AVR Pin       | Direction  | APC DB-9 Pin       | 
|---------------|---------------|------------|--------------------|
| Battery Power | 12 (Port B 0) | To AVR     | 2                  |
| Mains Down    | 13 (Port B 1) | To Arduino | 3 (Open Collector) |
| Ground        | 10 (Gnd)      | n/a        | 4                  |
| Low Battery   | 14 (Port B 2) | To Arduino | 5 (Open Collector) |
| Mains Up      | 15 (Port B 3) | To Arduino | 6 (Open Collector) |
| Power State   | 5 (Port A 0)  | To APC     | 7                  |

(thanks to [APCUPSD](http://apcupsd.org/manual/manual.html#voltage-signalling) for this info, made life a lot easier!)
