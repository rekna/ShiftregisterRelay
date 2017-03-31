# ShiftregisterRelay
Switch Relays with a Shiftregister and save power

Based on https://www.facebook.com/notes/236954946446659/The%20C-Programming-Challenge%20(2)/868068343335313/

To reduce power consumtion when switching relays on ready made relay modules from about 60mA/port (when driven permanently to GND), propagation delay of relays on changes to the input ports can be utilized to reduce "on" times to a minimum, hence less power consumption to drive the relais.

Prerequesitions

This lib timing is based on the relay modules using SONGLE SRD-05VDC-SL-C relays, but might be addapted to others, too. The test module consisted of 4x relay channels, switch signal active LOW.

According to the datasheet:

Turn On Time is : max. 10ms
    => So a change at relay state requires boost of 10ms
    => Use 50% more to be sure => 15ms
    => To keep things simple, this boost will be done on any change, although it is needed only when relay are turned on
Turn Off Time is: max. 5ms

Measured experimental times:
Turn On Time Relay									          approx. 5,4 ms
Turn Off Time Relay									          approx. 4,8 ms
Allowed pause time without relay reset:				approx. 2ms (max value)
Needed refresh time to keep relay alive:			approx. 1ms (min value) => This leads to max. refresh Ratio of 1:2 (on:off)

Code Time for Shiftregister Load					approx. 12 us (16MHz AVR)
Code Time ISR total (around 300 clocks) 			approx. 19 us => 1,9% (16MHz) uC time utilization

ISR will be called every 1ms (time measurement + SR load)

Mask definitions for 8 bit mask:
Sustain relay state: 2 bits on / 1 bit off (extrem condition)		=> B00100101 = 0x25 (approx. 12 mA / relay)
- (reflects to measured extrem timings above for refresh; single relays might not switch state properly)
Sustain relay state: 1 bit on / 1 bit off (save condition)			=> B10101010 = 0xAA (approx. 20 mA / relay)

Mask definitions for 16 bit mask (provides more granular timing):
Sustain relay state: 6 bits on / 10 bits off (extrem condition)		=> B0100101001010010 = 0x2495 (ca. 12 mA / relay)
- (reflects to measured extrem timings above for refresh; single relays might not switch state properly)
Sustain relay state: 7 bits on / 9 bits off (working condition)		=> B0010101001010101 = 0x2A55 (ca. 16 mA / relay)
- (provides a little bit more power than measured above; worked for my test module)
Sustain relay state: 8 bits on / 8 bits off (save condition)			=> B1010101010101010 = 0xAAAA (ca. 20 mA / relay)

Disclaimer:
The author is not responsible for any damage that may be caused to relay board, connected devices, or other electronics by using this library.
