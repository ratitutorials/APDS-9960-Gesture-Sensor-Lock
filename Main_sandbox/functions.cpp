/*
 * functions.cpp
 *
 *  Created on: Feb 6, 2016
 *      Author: dean__000
 */
#include <msp430.h>
#include "functions.h"

// Defines
#define LED_PIN BIT0

void wait_time(unsigned long delayCycles){
	//__disable_interrupt();
	volatile unsigned long i;
	for(i = 0; i < (delayCycles * 32); i++);
	//__enable_interrupt();
}

void blink_LED(uint8_t reps, unsigned long msec){
	int i;
	for(i=0; i<reps; i++){
		P2OUT &= ~LED_PIN;
		wait_time(msec);
		P2OUT |= LED_PIN;
		wait_time(msec);
	}
}

void uart_transmit(char *st, uint8_t len){
	uint8_t i;
	for(i=0; i < len; i++){
		while  (!(IFG2&UCA0TXIFG));			// Wait for buffer to be ready to send more data
		UCA0TXBUF = *st;                 // TX next character
		st++;
	}
}

/*
void wait_time(unsigned int delayCycles){
	TACCTL0 |= CCIE;	// Enables interrupt for timer
	TACCR0 = delayCycles * 28;	// ACLK runs at 32kHz.  Multply 32 by delayCycles and you've got yourself a timer in miliseconds!
	TACTL |= TASSEL_1;	//Select ACLK as Timer interrupt source (ACLK is the only clock that works in LPM3!)
	TACTL |= MC_1;	// Mode 1, tells the interrupt to count UP to TACCR0 then trigger the interrupt
	__bis_SR_register(LPM3_bits + GIE);	// Put into LPM3 "Low Power Mode 3" with GIE enabled "Global interrupts"
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void){
	TACCTL0 &= ~CCIE;	// Disable timer interrupt from capturing ALL other interrupts!
	__bic_SR_register_on_exit(CPUOFF);	// Exit low power mode
}
*/
#pragma vector = PORT2_VECTOR
__interrupt void Port_2(void){
	//P2IE &= ~BIT1;
	isr_flag = true;
	P2IFG &= ~BIT1;	// Clear interrupt on pin
	__bic_SR_register_on_exit(CPUOFF);
}
