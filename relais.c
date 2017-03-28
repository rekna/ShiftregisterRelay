/*
 * relais.c
 *
 * Created: 27.03.2017 21:32:40
 *  Author: Reiner Knapp (sehrknapp@yahoo.de)
 */ 

#include <relais.h>

// Variablen Relaissteuerung
volatile uint16_t maskS=SR_RELAIS_SUSTAIN_MASK;				// Maske für Relaiserhaltung
volatile uint8_t maskChangeSteps=0;							// Zaehler für Burstdauer
volatile uint8_t srPattern=0;								// Speicher für Relaisausgänge

// Zeit Funktionen
volatile uint32_t _timerMillis;								// Vergangene Millisekunden
volatile uint32_t _timerMicros;								// Vergangene Mikrosekunden
uint32_t srRefreshTimestamp;								// Letzte manuelle Aktualisierung Schieberegister

//
// Timer Routinen
//

// Timer einrichten
void initTimer () {
	// Timer 0 Ausführung alle 1 ms
	// F_CPU = 16000000
	// Prescaler 64
	// Ticks=16000000/64 = 250000
	// 1 IRQ = 1 MS => 250000/1000 => 250 CTC Grenze (16 MHz); 125 CTC Grenze (8 MHz)

#ifdef F_CPU=16000000
	OCR0A=250;
#endif
#ifdef F_CPU=8000000
	OCR0A=125;
#endif
	// CTC Mode einschalten (WGM21)
	TCCR0A|=(1<<WGM01);
	// IRQ Einschalten
	TIMSK0|=(1<<OCIE0A);
	// Prescaler auf 64 (CS22, CS20)
	TCCR0B=(1<<CS01)|(1<<CS00);

	// Zähler zurücksetzen
	_timerMillis=0;
	_timerMicros=0;

	// IRQ einschalten	
	sei();

}

// Timer ISR (Aufruf alle 1ms)
ISR(TIMER0_COMPA_vect) {
	// bereits anliegende Schieberegsiter Daten latchen
	srLatch();
	// Timerzähler setzen
	_timerMillis++;
	_timerMicros+=1000;
	// nächste Schieberegister Daten vorloaden, für entspannteres Timing
	srPreload(srPattern);
}

// Aktuelle Millis
uint32_t millis() {
	return _timerMillis;
}

// Aktuelle Micros
uint32_t micros() {
#ifdef F_CPU=16000000
	// Laufende Micros mit 4 Multiplizieren, da nur 250 Ticks gezählt werden
	return _timerMicros+(TCNT2<<2);
#endif
#ifdef F_CPU=8000000
	// Laufende Micros mit 8 Multiplizieren, da nur 125 Ticks gezählt werden
	return _timerMicros+(TCNT2<<4);
#endif
}

// Warte x ms
void delay(uint32_t msec) {
	uint32_t start=millis();
	while ((millis()-start)<msec) ;
}

// Warte x us
void delayus(uint32_t usec) {
	uint32_t start=micros();
	while ((micros()-start)<usec) ;
}

//
// Schieberegister Verwaltung
//

// Schieberegister init
void relayInit () {
	// SR Init
	// Portausgabe einstellen
	SR_DDR|=(SR_PIN_CLK|SR_PIN_DO|SR_PIN_STO);
	// SR Clock, DataOut, Store auf LOW
	SR_PORT&=~(SR_PIN_CLK|SR_PIN_DO|SR_PIN_STO);
	// Alle Ausgänge aus
	relaySet(0);
	srPreload(srPattern);
	srLatch();
}

// Datenbits in Schieberegister laden
void srPreload (uint8_t data) {
	// Byte maskieren, wenn kein Burst
	if (maskChangeSteps==0) {
		// Ausgabe maskieren
		data=data&(maskS&0xff);
	} else {
		// Burstzähler verringern
		maskChangeSteps-=maskChangeSteps>0?1:0;
	}
	for (uint8_t i=0; i<8; i++) {
		// Clock low
		SR_PORT&=~SR_PIN_CLK;
		// Set data
		if ((data&0x01)==0)
			SR_PORT|=SR_PIN_DO;
		else
			SR_PORT&=~SR_PIN_DO;
		// Clock high
		SR_PORT|=SR_PIN_CLK;
		// Shift Bits
		data=data>>1;
	}
	// Maske rotieren
	uint8_t msbS=(maskS&0x8000)==0?0:1;
	maskS=(maskS<<1)|msbS;
}

// Neues Relaismuster einstellen
void relaySet (uint8_t data) {
	// Liegt Änderung bei einem Relais vor? => Burst Sequenz einstreuen
	if (data!=srPattern) {
		maskChangeSteps=SR_RELAIS_BURST_CYCLES;
	}
	// Daten abspeichern
	srPattern=data;
}

// Anliegende Schieberegisterdaten an Ausgänge latchen
void srLatch () {
	SR_PORT&=~SR_PIN_STO;
	SR_PORT|=SR_PIN_STO;
}

// Manuelles Refresh des Schieberegisters (für IRQ lose Ausführung)
void relayRefresh () {
	// Zeit für Refresh?
	if (micros()-srRefreshTimestamp>=SR_RELAIS_CYCLE_TIME) {
		srRefreshTimestamp=micros();
		srPreload(srPattern);
		srLatch();
	}
}

