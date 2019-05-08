// put implementations for functions, explain how it works
// put your names here, date
#include "../inc/tm4c123gh6pm.h"

// ADC.c
// Runs on LM4F120/TM4C123
// Provide functions that initialize ADC0
// Last Modified: 11/6/2018
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ volatile int delay; 
//*** students write this ******
	SYSCTL_RCGCGPIO_R |= 0x08 ; 		// enable Port D 
	delay = 0; 
	delay =0;
	delay=0;
	delay=0;
	delay=0;
	GPIO_PORTD_DIR_R &= ~0x04	; 		// Port D is input 
	GPIO_PORTD_AFSEL_R |= 0x04; 		
	GPIO_PORTD_DEN_R &= ~0x04	;			// disable Port D digital 
	GPIO_PORTD_AMSEL_R |= 0x04; 		// enable analog 
	
	SYSCTL_RCGCADC_R |= 0x01	; 		// enable	ADC clock 
	delay = 1; 
	delay = 2; 
	delay = 3; 
	delay = 4; 
	delay = 5; 
	delay = 6; 
	delay = 7; 
	delay = 8;
	delay = 9; 
	delay = 10; 
	delay = 11; 
	delay = 12;
	ADC0_PC_R |= 0x01; 							// 125k HZ priority 
	ADC0_SSPRI_R = 0x0123; 				// seq3 is highest priority 
	ADC0_ACTSS_R &= ~0x08;					// disable seq3 
	ADC0_EMUX_R &= ~0xF000; 				// seq3 is software trigger 
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0xFFFFFFF0) + 5; // Ain5, PD2 
	ADC0_SSCTL3_R = 0x0006; 
	ADC0_IM_R &= ~0x08; 
	ADC0_ACTSS_R |= 0x08; 
	ADC0_SAC_R = 0x06;

}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
//*** students write this ******
	uint32_t data; 
	ADC0_PSSI_R = 0x0008 ;		// start ADC 
	
	while ((ADC0_RIS_R&0x0008) == 0) {} 
	
	data = ADC0_SSFIFO3_R&0xFFF; 
		
	ADC0_ISC_R =	0x0008		; 

  return data;
}





