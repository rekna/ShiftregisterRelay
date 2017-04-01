/*
 * relais.c
 *
 * Created: 27.03.2017 21:32:40
 *  Author: Reiner Knapp (sehrknapp@yahoo.de)
 */ 

#include <relais.h>

// Variables relay control
volatile uint16_t maskS=SR_RELAIS_SUSTAIN_MASK;				// maske for relay refresh
volatile uint8_t maskChangeSteps=0;							// counter burst drive
volatile uint8_t srPattern=0;								// relay control bits

// time control
volatile uint32_t _timerMillis;								// passed milli seconds
volatile uint32_t _timerMicros;								// passed micro seconds
uint32_t srRefreshTimestamp;								// last manual SR refresh

//
// timer routines
//

// timer setup
void initTimer () {
	// Timer 0 execution every 1 ms
	// F_CPU = 16000000
	// Prescaler 64
	// Ticks=16000000/64 = 250000
	// 1 IRQ = 1 MS => 250000/1000 => 250 CTC Border (16 MHz); 125 CTC Border (8 MHz)

#ifdef F_CPU=16000000
	OCR0A=249;
#endif
#ifdef F_CPU=8000000
	OCR0A=124;
#endif
	// CTC Mode on (WGM21)
	TCCR0A|=(1<<WGM01);
	// IRQ enable
	TIMSK0|=(1<<OCIE0A);
	// Prescaler to 64 (CS22, CS20)
	TCCR0B=(1<<CS01)|(1<<CS00);

	// counter init
	_timerMillis=0;
	_timerMicros=0;

	// General IRQ enable
	sei();

}

// Timer ISR
ISR(TIMER0_COMPA_vect) {
	// latch preloaded shiftregister data
	srLatch();
	// increase timer counter
	_timerMillis++;
	_timerMicros+=1000;
	// preload next shiftregister data for timing reasons
	srPreload(srPattern);
}

// current millis
uint32_t millis() {
	return _timerMillis;
}

// current micros
uint32_t micros() {
#ifdef F_CPU=16000000
	// micro counter multiplied by 4, only counted every 250 ticks
	return _timerMicros+(TCNT2<<2);
#endif
#ifdef F_CPU=8000000
	// micro counter multiplied by 8, only counted every 125 ticks
	return _timerMicros+(TCNT2<<4);
#endif
}

// wait x ms
void delay(uint32_t msec) {
	uint32_t start=millis();
	while ((millis()-start)<msec) ;
}

// wait x us
void delayus(uint32_t usec) {
	uint32_t start=micros();
	while ((micros()-start)<usec) ;
}

//
// shift register management
//

// relay control init
void relayInit () {
	// shift register init
	// Setup port pins
	SR_DDR|=(SR_PIN_CLK|SR_PIN_DO|SR_PIN_STO);
	// shift register clock, dataOut, dtore to LOW
	SR_PORT&=~(SR_PIN_CLK|SR_PIN_DO|SR_PIN_STO);
	// All relay output disable
	relaySet(0);
	srPreload(srPattern);
	srLatch();
}

// load data bits to shift register
void srPreload (uint8_t data) {
	// mask data byte, if not burst mode
	if (maskChangeSteps==0) {
		// mask data byte
		data=data&(maskS&0xff);
	} else {
		// descrease burst counter, do not mask output
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
	// rotate mask
	uint8_t msbS=(maskS&0x8000)==0?0:1;
	maskS=(maskS<<1)|msbS;
}

// Save new Relay state
void relaySet (uint8_t data) {
	// Is there a change in the relay state? => Init burst sequenz
	if (data!=srPattern) {
		maskChangeSteps=SR_RELAIS_BURST_CYCLES;
	}
	// Save new relay pattern
	srPattern=data;
}

// latch shift register to outputs
void srLatch () {
	SR_PORT&=~SR_PIN_STO;
	SR_PORT|=SR_PIN_STO;
}

// manual refresh of shift register (for IRQ less execution)
void relayRefresh () {
	// time passed for refresh?
	if (micros()-srRefreshTimestamp>=SR_RELAIS_CYCLE_TIME) {
		srRefreshTimestamp=micros();
		srPreload(srPattern);
		srLatch();
	}
}

