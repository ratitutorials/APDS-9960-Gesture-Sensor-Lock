#include <msp430.h>
#include <stdint.h>
#include "I2C.h"
#include "SparkFun_APDS9960.h"
#include "functions.h"
#include "sha2.h"

bool handleGesture(void);
void uart_transmit(char *st, uint8_t len);
void uart_setup();
void led_setup();
void gesture_setup();
void isr_setup();
void SHA2_hash(uint32_t *pwdvalue, uint32_t * hash);
void hash_print();
bool hash_compare(const uint8_t *pwdValue, uint8_t bit);
void solenoid(uint8_t bit);


//Globals
MSP430_I2C Wire = MSP430_I2C();
typedef struct password {
	  uint8_t value[12];
	  uint8_t counter;
} password;

// Global Variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
char gValue;
uint8_t rxbyte;
struct password *passwordPTR;
uint32_t * pwd;
uint32_t hash[8];
const uint8_t pwd1[] = "03379BEF0820673543DB8C37AC0DFF90CDF5CF2FFBBA1EB5F13574A7A6B65C10";	// Short
const uint8_t pwd2[] = "9032C6DBECCE08D696C10F9CC3A0F06B6C98BE42FE6D7708DDEFA09B78AF283D";   // Long

void setup(){

	  //const unsigned char device_address = 0x39;
	  WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT

	  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO to 1Mhz
	  DCOCTL = CALDCO_1MHZ;

	  P2DIR = BIT3 + BIT4;
	  P2OUT &= ~BIT3;
	  P2OUT &= ~BIT4;

	  led_setup();
	  uart_setup();
	  // Print banner
	  uart_transmit("------\n\r", 8);
	  uart_transmit("SparkFun APDS-9960 - GestureTest\n\r", 34);
	  uart_transmit("------\n\r", 8);

	  gesture_setup();
	  isr_setup();


}

void SHA2_hash(uint32_t *pwdvalue, uint32_t * hash){
	uint32_t message[16];
	  uint32_t bytes_to_be_hashed;
	  short hash_mode;

	  // Set message
	  message[0]=0xc98c8e55;

	  bytes_to_be_hashed = 4;
	  hash_mode = SHA_256;

	SHA_2( pwdvalue, bytes_to_be_hashed, hash, hash_mode);
}

void gesture_setup(){
	if ( apds.init() ) {
		    uart_transmit("APDS-9960 initialization complete\n\r", 35);
	} else {
		uart_transmit("Something went wrong during APDS-9960 init!\n\r", 45);
		P2OUT |= BIT0;
		while(1);
    }


	// Start running the APDS-9960 gesture sensor engine
	  if ( apds.enableGestureSensor(true) ) {
		uart_transmit("Gesture sensor is now running\n\r", 31);
	  } else {
		uart_transmit("Something went wrong during gesture sensor init!\n\r", 50);
		P2OUT |= BIT0;
		while(1);
	  }
}

void isr_setup(){
	// Configure interrupt for P2.1
	P2SEL &= ~BIT1;
	P2DIR &= ~BIT1;	// P2.1 is an input
	//P2REN |= BIT1; // Set internal pull-up resistor
	P2IES |= BIT1;  // Interrupt set as Falling-edge
	P2IFG &= ~BIT1;
	P2IE |= BIT1; // Enable interrupt
	gValue = 0x00;
}

void led_setup(){
	P2DIR |= BIT0;
    P2OUT |= BIT0;
}

void uart_setup(){
	P1SEL |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
	P1SEL2 |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD

	UCA0CTL1 |= UCSSEL_2;                     // CLK = MSCLK
	UCA0BR0 = 0x68;                           // 1MHz/104 = 9600
	UCA0BR1 = 0x00;                           //
	UCA0MCTL = UCBRS1 + UCBRS0;               // Modulation UCBRSx = 3
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}

void solenoid(uint8_t bit){
	P2OUT |= bit;
	wait_time(2000);
	P2OUT &= ~bit;
}

int main()
{

  //Variables

  uint32_t passwordNUM = 0x00;
  pwd = &passwordNUM;
  struct password passW = {
		  { 0 },
		  0
  };

  passwordPTR = &passW;

  setup();

  //SHA2_hash(&hash[0]);
  blink_LED(1, 1000);
  while(1){

	  __bis_SR_register(LPM4_bits + GIE);
	  if( isr_flag ) {

	      handleGesture();
		  uart_transmit(&gValue, 1);
	      isr_flag = false;
	   }
  }

}


bool handleGesture() {
    if ( apds.isGestureAvailable() ) {
    uint8_t currentGesture = apds.readGesture();

    switch ( currentGesture ) {
      case DIR_UP:
        uart_transmit("UP\n\r", 4);
        blink_LED(1, 300);
        break;
      case DIR_DOWN:
        uart_transmit("DOWN\n\r", 6);
        blink_LED(1, 300);
        break;
      case DIR_LEFT:
        uart_transmit("LEFT\n\r", 6);
        blink_LED(1, 300);
        break;
      case DIR_RIGHT:
        uart_transmit("RIGHT\n\r", 7);
        blink_LED(1, 300);
        break;
      case DIR_NEAR:
        uart_transmit("NEAR\n\r", 6);
        blink_LED(2, 100);
        break;
      case DIR_FAR:
        uart_transmit("FAR\n\r", 5);
        blink_LED(5, 50);
        break;
      default:
        uart_transmit("NONE\n\r", 6);
        blink_LED(2, 50);
    }

    if (currentGesture >= 1 && currentGesture <= 4){	// Range between 1-4
    	if ((passwordPTR->counter) >= 12) {  // Cycle the counter back to the beginning
    		passwordPTR->counter = 0;
    	}
    	passwordPTR->value[passwordPTR->counter] = currentGesture;
    	passwordPTR->counter++;
    	*pwd = ((*pwd << 2) | (currentGesture - 1));
    }

    if (currentGesture == 6){
    	SHA2_hash(pwd, &hash[0]);
    	hash_print();
    	uart_transmit("\n\r", 2);

    	if(hash_compare(pwd1, BIT3)){
    	    	uart_transmit("True\n\r", 6);
    	    }
    	    else{
    	    	uart_transmit("False\n\r", 7);
    	    }

    	    if(hash_compare(pwd2, BIT4)){
    	        	uart_transmit("True\n\r", 6);
    	        }
    	   else{
    	        	uart_transmit("False\n\r", 7);
    	    }

    }





    return true;
  }
  return false;
}

void hash_print(){

	int i;
	int j;
	char hexVal = 0x0;
	for(i=0; i<8; i++){
		for(j=7; j>=0; j--){
		  hexVal = (hash[i] >> j*4) & 0xF;
		  if (hexVal < 0xA){
			  hexVal += 48;
		  }
		  else{
			  hexVal += 55;
		  }
		  uart_transmit(&hexVal, 1);
		}
	}
	uart_transmit("\n\r",2);
	*pwd = 0x00;	// Clear password
}

bool hash_compare(const uint8_t *pwdValue, uint8_t bit){
	int i;
	int j;
	char hexVal = 0x0;
	for(i=0; i<8; i++){
		for(j=7; j>=0; j--){
		  hexVal = (hash[i] >> j*4) & 0xF;
		  if (hexVal < 0xA){
			  hexVal += 48;
		  }
		  else{
			  hexVal += 55;
		  }

		  if (hexVal != *pwdValue){
			  return false;
		  }
		  pwdValue++;
		}
	}
	solenoid(bit);
	return true;
}

