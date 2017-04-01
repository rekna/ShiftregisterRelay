/*
 * relais.h
 *
 * Created: 27.03.2017 21:32:53
 *  Author: Reiner Knapp (sehrknapp@yahoo.de)
 */ 


#ifndef RELAIS_H_
#define RELAIS_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

// Port Pins Schieberegister
#define SR_PORT		PORTD
#define SR_DDR		DDRD
#define SR_PIN		PIND
#define SR_PIN_CLK	(1<<4)
#define SR_PIN_DO	(1<<3)
#define SR_PIN_STO	(1<<2)

// Prerequesitions
//
// Tested Relay: SONGLE SRD-05VDC-SL-C
//
// Turn On Time (datasheet)	: max. 10ms => Change at relay state requires boost of 10ms => Use 150% to be sure => 15ms
// Turn Off Time (datasheet): max. 5ms
//
// Measured experimental times
// Turn On Time Relay									ca. 5,4 ms
// Turn Off Time Relay									ca. 4,8 ms
// Allowed pause time without relay reset:				ca. 2ms (max value)
// Needed refresh time to keep relay alive:				ca. 1ms (min value) => Refresh Ratio is 1:2 (on:off)
//
// Time for Shiftregister Load							ca. 12 us (16 MHz)
// Time ISR total (ca. 300 clocks)					ca. 19 us => 1,9% (16MHz) uC time use
//
// ISR will be called every 1ms (time measurement + SR load)
//

// Mask definition for 8 bit mask
//
// Sustain relay state: 2 bits on / 1 bit off (extrem condition)		=> B00100101 = 0x25 (ca. 12 mA / relay) (reflects to measured timings above for refresh; single relays might not switch state)
// Sustain relay state: 1 Bit on / 1 Bit off (save condition)			=> B10101010 = 0xAA (ca. 20 mA / relay)

// Mask definition for 16 bit mask (provides more granular timing)
//
// Sustain relay state: 6 bits on / 10 bits off (extrem condition)		=> B0100101001010010 = 0x2495 (ca. 12 mA / relay) (reflects to measured timings above for refresh; single relays might not switch state)
// Sustain relay state: 7 bits on / 9 bits off (working condition)		=> B0010101001010101 = 0x2A55 (ca. 16 mA / relay) (power a little bit more than measured above; worked for my test)
// Sustain relay state: 8 bits on / 8 bits off (save condition)			=> B1010101010101010 = 0xAAAA (ca. 20 mA / relay)

// Relay control definition
#define SR_RELAIS_BURST_CYCLES	15					// Burst drive for relays in ms
#define SR_RELAIS_SUSTAIN_MASK	0x2A55				// bit mask for pulsed relay control
#define SR_RELAIS_CYCLE_TIME	1000				// 1 ms cycle time

// Function deklarationen

void initTimer();
uint32_t millis();
uint32_t micros();
void delay(uint32_t msec);
void delayus(uint32_t usec);
void relayInit ();
void relaySet (uint8_t data);
void relayRefresh ();

#endif /* RELAIS_H_ */
