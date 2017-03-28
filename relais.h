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

// Vorüberlegungen
//
// Verwendetes Relais: SONGLE SRD-05VDC-SL-C
//
// Einschaltzeit (Datenblatt): max. 10ms => Änderung Relais Zustand erfordert Stromburst von 10ms Dauer => Verwende 50% Sicherheitsaufschlag => 15ms
// Ausschaltzeit (Datenblatt): max. 5ms
//
// Experimentell ermittelte Zeiten
// Einschaltzeit Relais									ca. 5,4 ms
// Ausschaltzeit Relais									ca. 4,8 ms
// Erlaubte Pausen Zeit ohne Rücksetzen Relais:			ca. 2ms (Maximalwert)
// benötigte Zeit Ansteuerung Relais für Refresh:		ca. 1ms (Minimalwert) => Erhaltungsverhältnis von 2:1 (on:off)
//
// Dauer für Schieberegister Ausgabe					ca. 4 us
// Dauer ISR gesamt (ca. 175 Takte)						ca. 11 us => 1,1% (16MHz) uC Zeit
//
// Takt für Schieberegister setzen => 1ms (entspricht Timer IRQ)
//

// Maskendefinition für 8 Bit Maske
//
// Erhalten Relais Zustände: 2 Bit an / 1 Bit aus (Extrem Zustand)		=> B00100101 = 0x25 (ca. 12 mA / Relais) (entspricht oben gemessenen maximalen Timingwerten für Erhalt; evtl. bleibt Relais nicht sicher an)
// Erhalten Relais Zustande: 1 Bit an / 1 Bit aus (sichere Versorgung)	=> B10101010 = 0xAA (ca. 20 mA / Relais)

// Maskendefinition für 16 bit Maske (ermöglichst feinere 
//
// Erhalten Relais Zustände: 6 Bit an / 10 Bit aus (Extrem Zustand)		=> B0100101001010010 = 0x2495 (ca. 12 mA / Relais) (entspricht oben gemessenen maximalen Timingwerten für Erhalt; evtl. bleibt Relais nicht sicher an)
// Erhalten Relais Zustände: 7 Bit an / 9 Bit aus (Grenz Zustand)		=> B0010101001010101 = 0x2A55 (ca. 16 mA / Relais) (knapp Unterhalt der oben gemessenen Timingwerten für Erhalt; Test mit allen Relais hat funktioniert)
// Erhalten Relais Zustande: 8 Bit an / 8 Bit aus (sichere Versorgung)	=> B1010101010101010 = 0xAAAA (ca. 20 mA / Relais)

// Steuerdefinitionen für Relaisansteuerung
#define SR_RELAIS_BURST_CYCLES	15					// Vollansteuerung Relais in ms (s.o.)
#define SR_RELAIS_SUSTAIN_MASK	0x2A55				// Bitmaske für getaktete Relaisansteuerung
#define SR_RELAIS_CYCLE_TIME	1000				// 1 ms cycle time

// Funktionsdeklarationen

void initTimer();
uint32_t millis();
uint32_t micros();
void delay(uint32_t msec);
void delayus(uint32_t usec);
void relayInit ();
void relaySet (uint8_t data);
void relayRefresh ();

#endif /* RELAIS_H_ */