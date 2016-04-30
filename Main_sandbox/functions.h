/*
 * functions.h
 *
 *  Created on: Feb 6, 2016
 *      Author: dean__000
 */
#include <stdint.h>

#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

volatile bool isr_flag;

void blink_LED(uint8_t reps, unsigned long msec);
void wait_time(unsigned long delayCycles);
void uart_transmit(char *st, uint8_t len);


#endif /* WORKING_FUNCTIONS_H_ */
