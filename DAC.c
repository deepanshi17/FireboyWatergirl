// Danica Corbita & Deepanshi Sharma 
// put your names here, date
#include "../inc/tm4c123gh6pm.h"
#include <stdint.h> 


//****Dac_Init****// 
// Initializes the DAC for PortB// 
void Dac_Init (void) { 
	 volatile int delay; 
	SYSCTL_RCGCGPIO_R |= 0x02; // enable clock for PORT B 
	delay = 0; // let clock run 
	GPIO_PORTB_AMSEL_R &= ~0x0F;	// no analog
	GPIO_PORTB_PCTL_R &= ~0x0000FFFF;	// regular function
	GPIO_PORTB_DIR_R |= 0x0F; // output on PORTB 
	GPIO_PORTB_AFSEL_R &= ~0x0F;	//disable alt function
	GPIO_PORTB_DEN_R |= 0x0F; //
	
}

void DAC_Out (uint32_t data) {
	GPIO_PORTF_DATA_R ^= 0x08; 
	GPIO_PORTB_DATA_R = (data & 0x0F); 
	
}


