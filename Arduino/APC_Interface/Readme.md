DumbUPS - Firmware for APC Dumb ups interface
=====================

This repo contains a platformio project to provide a DumbUPS serial interface for a APC UPS that uses the dumb interfaces

The circuit for this is [here](Schematic.pdf)

Pinouts
=

| Signal        | Arduino Pin | Direction  | APC DB-9 Pin        |
|---------------|-------------|------------|---------------------|
| Battery Power | 8           | To Arduino | 2                   |
| Mains Down    | 10          | To Arduino | 3 (Open Collector)  |
| Ground        | Gnd         | n/a        | 4                   |
| Low Battery   | 9           | To Arduino | 5 (Open Collector)  |
| Mains Up      | 11          | To Arduino | 6  (Open Collector) |
| Power State   | 12          | To APC     | 7                   |

(thanks to [APCUPSD](http://apcupsd.org/manual/manual.html#voltage-signalling) for this info, made life a lot easier!)
