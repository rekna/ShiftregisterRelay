/*
 *
 * Created: 26.03.2017 17:25:52
 * Author : Reiner
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <util/delay.h>
#include <relais.h>

uint32_t timestamp=0;		// time stamp variable
uint8_t relIndex=0;			// relay state

bool toggle=false;			// relay on/off flag

int main(void)
{
	relayInit();

	initTimer();

	timestamp=millis();
	relIndex=0x01;
    /* Replace with your application code */
    while (1) 
    {
		if ((millis()-timestamp)>5000) {
			timestamp=millis();
			// switch relay every x sec on/off
			if (toggle) {
				relaySet(0);
			} else {
				// true = switch single relay one after the other
				// false = switch relays binary
				if (true) {
					relaySet(relIndex);
					relIndex=relIndex<<1;
					if (relIndex==0) relIndex=0x01;
				} else {
					relaySet(relIndex++);
					relIndex&=0xf;
				}
			}
			toggle=!toggle;
		}
    }
}

