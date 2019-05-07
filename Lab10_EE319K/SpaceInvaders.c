// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 11/20/2018 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2018

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2018

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "TExaS.h"  
#include "PLL.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "DAC.h" 
#include "Print.h" 


#define period (7256)
#define A (5682)
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))

uint32_t signal=0;
uint32_t ADCMail=0;
int ADCStatus=0;
char units[] = " cm";
char *ptr = &units[0];
uint32_t Data;        // 12-bit ADC
uint32_t Position;    // 32-bit fixed-point 0.001 cm

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts


void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0; 						
	NVIC_ST_RELOAD_R = 666800; 
	NVIC_ST_CURRENT_R = 0; 
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000;
	NVIC_ST_CTRL_R = 0x07; 
}

void SysTick_Handler(void){
	PF2 ^= 0x04;      // Heartbeat
	PF2 ^= 0x04;      // Heartbeat
	ADCMail = ADC_In();
	ADCStatus=1;
	PF2 ^= 0x04;      // Heartbeat
}
//***PortFInit***// 
// initializes PortF LEDs to work as heartbeats // 
// PF1 LED - for music interrupts (timers)			// 
// PF2 LED - for ADC interrupt	(systick)				// 					
void PortFInit (void) { volatile int delay; 
	
	SYSCTL_RCGCGPIO_R |= 0x20; // clock for PORT F
	delay = 0; 
	GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY ; 
	GPIO_PORTF_CR_R |= 0xFF	;
	GPIO_PORTF_DIR_R |= 0x0E; //output LED PF123
	GPIO_PORTF_DEN_R |= 0x0E; // enable PF1, PF2, PF3 

}
//*****ButtonsInit*****// 
// Initializes PE0, PE1 as buttons // 
void ButtonsInit(void){ volatile int delay; 
	SYSCTL_RCGCGPIO_R |= 0x10; // enable clock for PORT E
	delay = 0; // let clock run 
	GPIO_PORTE_DIR_R &= ~0x03; // inputs on PORTE
	GPIO_PORTE_DEN_R |= 0x03; // enable 
} 

uint32_t Convert(uint32_t input){
	Position = ((input*446)/1000)-487;
  return(Position);
}

int main(void){
	PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
	PortFInit() ; 
	SysTick_Init();
	ADC_Init();
	ST7735_InitR(INITR_REDTAB);
 
	Sound_Init(); 						// initializes sound (& DAC as well) 
	ButtonsInit(); 
 
	uint32_t button = 0; 
	uint32_t lastbutton = 0;

//  Output_Init();
// ST7735_FillScreen(0x0000);            // set screen to black
//	
	Timer1_Init(&PlayBackgroundMusic, A); 
	EnableInterrupts(); 
//	
//	ST7735_FillScreen(0x0000); 
//	ST7735_DrawBitmap(64, 80, Firegirl, 17, 35);  
	
	
  while(1){
		
//		Data = ADC_In(); 	// obtain horizontal motion data from ADC (x)
//		Position = Convert(Data); 
//		ST7735_SetCursor(0,0); 
		
	if(ADCStatus==1){
		signal = ADCMail;
		ADCStatus=0;
		signal = Convert(signal);
		ST7735_OutString("    "); 
    ST7735_SetCursor(6,0);
		LCD_OutFix(signal);
		ST7735_OutString(ptr); 
	} 
		// button interrupts ///					
		 button = (GPIO_PORTE_DATA_R & 0x03); 
		if (((button == 1) && (lastbutton == 0)) || ((button == 2) && (lastbutton == 0)))	// either button pressed 
			{
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayGrunt();
		}
		 else if ( button == lastbutton) { } 				// no buttons pressed 
			else {
					PlayNothing(); 
					TIMER1_CTL_R = 0x00000001;
			} 
			lastbutton = button ; 
			
  }

}





