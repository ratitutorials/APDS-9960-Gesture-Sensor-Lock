/*
 * I2C.c
 *
 *  Created on: Feb 4, 2016
 *      Author: dean__000
 */

#include <msp430.h>
#include "I2C.h"
#include <stdint.h>

MSP430_I2C::MSP430_I2C()
{
	TXByteCtr = 0;
	Rx = 0;
}

MSP430_I2C::~MSP430_I2C()
{

}


//-------------------------------------------------------------------------------
// The USCI_B0 data ISR is used to move received data from the I2C slave
// to the MSP430 memory. It is structured such that it can be used to receive
//-------------------------------------------------------------------------------
#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
  if(Rx == 1){                              // Master Recieve?
	*RXBytePtr = UCB0RXBUF;                       // Get RX data
	__bic_SR_register_on_exit(CPUOFF);

  }

  else if(Rx == 2){
	 RXBlockCtr--;                            // Decrement TX byte counter
	if (RXBlockCtr){                            // Check TX byte counter
		*RXBytePtr = UCB0RXBUF;                       // Get RX data
		RXBytePtr++;	// Increment pointer if it's not at the end yet.
		if (RXBlockCtr == 1) UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
	}
	else{
		//UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
		*RXBytePtr = UCB0RXBUF;                       // Get RX data
		//IFG2 &= ~UCB0RXIFG;                     // Clear USCI_B0 TX int flag
		__bic_SR_register_on_exit(CPUOFF);
	}
  }

  else{                                     // Master Transmit
      if (TXByteCtr)                            // Check TX byte counter
        {
          UCB0TXBUF = *TXBytePtr;                     // Load TX buffer
          TXByteCtr--;                            // Decrement TX byte counter
          if (TXByteCtr) TXBytePtr++;		// Increment pointer if it's not at the end yet
          //IFG2 |= UCB0TXIFG;                     // Set USCI_B0 TX int flag
          //__bic_SR_register_on_exit(CPUOFF + GIE);
        }
        else
        {
          UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
          IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
          //__bic_SR_register_on_exit(CPUOFF);      // Exit LPM0 with interrupts enabled
          //UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);       // Clear interrupt flags
          __bic_SR_register_on_exit(CPUOFF);
        }
 }

}
void MSP430_I2C::init(void) {
	  P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
	  P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
      UCB0CTL1 |= UCSWRST;                      // Enable SW reset
      UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
      UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
      UCB0BR0 = 12;                             // fSCL = SMCLK/12 = ~100kHz
      UCB0BR1 = 0;
      //UCB0I2CSA = itgAddress;                         // Slave Address is 069h
      UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
      IE2 |= UCB0RXIE + UCB0TXIE;               //Enable RX and TX interrupt
}

void MSP430_I2C::transmit(uint8_t TxBcount, uint8_t *Txbytes, uint8_t device_address){
	//IE2 |= UCB0TXIE;               //Enable RX and TX interrupt
	Rx = 0;
	UCB0I2CSA = device_address;                         // Slave Address is 069h
	TXByteCtr = TxBcount;
	TXBytePtr = Txbytes;
    while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
    UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
    __bis_SR_register(LPM0_bits + GIE);        // Enter LPM0 w/ interrupts
}

void MSP430_I2C::receiveByte(uint8_t *store_val, uint8_t address, uint8_t device_address){
		//IE2 |= UCB0RXIE;               //Enable RX and TX interrupt
	    transmit(1, &address, device_address);	// Set location to retrieve data
		Rx = 1;
		RXBlockCtr = 1;
		RXBytePtr = store_val;
		UCB0I2CSA = device_address;
        while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
        UCB0CTL1 &= ~UCTR ;                     // Clear UCTR
        UCB0CTL1 |= UCTXSTT;                    // I2C start condition
        while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
        UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
        __bis_SR_register(LPM0_bits + GIE);        // Enter LPM0 w/ interrupts
}

void MSP430_I2C::receiveBlock(uint8_t len, uint8_t *store_val, uint8_t address, uint8_t device_address){
	    transmit(1, &address, device_address);
		Rx = 2;
		RXBytePtr = store_val;
		RXBlockCtr = len;
		UCB0I2CSA = device_address;
        while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
        UCB0CTL1 &= ~UCTR ;                     // Clear UCTR
        UCB0CTL1 |= UCTXSTT;                    // I2C start condition
        while (UCB0CTL1 & UCTXSTT);             // Start condition sent?
        __bis_SR_register(LPM0_bits + GIE);        // Enter LPM0 w/ interrupts
}

