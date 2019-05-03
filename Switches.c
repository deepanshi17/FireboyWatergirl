// Switches.c
// This software configures the off-board jump keys
// Program written by: Deepanshi Sharma and Danica Corbita
// Date Created: 5/2/2019
// Last Modified: 5/2/2019

// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// **************Switches_Init*********************
// Initialize piano key inputs, called once to initialize the digital ports
// Input: none 
// Output: none
void Switches_Init(void){ 
	volatile int delay; 
	SYSCTL_RCGCGPIO_R |= 0x10; // enable clock for PORT E
	delay = 0; // let clock run 
	GPIO_PORTE_DIR_R &= ~0x03; // inputs on PORTE
	GPIO_PORTE_DEN_R |= 0x03; // 
}

// **************Switches_In*********************
// Input from jump key inputs 
// Input: none 
// Output: 0 to 3 depending on keys
//   0x01 is just Key0, 0x02 is just Key1
//   bit n is set if key n is pressed
uint32_t Switches_In(void){
	 uint32_t output = (GPIO_PORTE_DATA_R & 0x03); 
	return output; 
}
