/*
 * I2C.h
 *
 *  Created on: Feb 4, 2016
 *      Author: dean__000
 */

#ifndef I2C_H_
#define I2C_H_

#include <msp430g2553.h>
#include <stdint.h>

int TXByteCtr;
uint8_t RXBlockCtr;
unsigned char PRxData;
int Rx;
unsigned char *TXBytePtr;
unsigned char *RXBytePtr;

class MSP430_I2C {
public:

	MSP430_I2C();
	~MSP430_I2C();
	void init(void);
	void transmit(uint8_t TxBcount, uint8_t *Txbytes, uint8_t device_address);
	void receiveByte(uint8_t *store_val, uint8_t address, uint8_t device_address);
	void receiveBlock(uint8_t len, uint8_t *store_val, uint8_t address, uint8_t device_address);

//private:
	//friend class SparkFun_APDS9960;
};





#endif /* WORKING_I2C_H_ */
