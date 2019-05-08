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
#include "PLL.h"
#include "ADC.h"
#include "Images.c" 
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "DAC.h" 
#include "Print.h" 


#define period (7256)
#define A (5682)
#define P (12000000)
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))

uint32_t signal=0;
uint32_t ADCMail=0;
uint32_t button = 0; 
uint32_t lastbutton = 0;
uint32_t Fall=0;
int ADCStatus=0;
int Bevo1F=0;
int Bevo2F=0;
int Bevo3F=0;
int Win=0;
int Stage1=0;
int Stage2=0;
int Stage3=0;
unsigned int Score=0;
char units[] = " cm";
char *ptr = &units[0];
uint32_t Data;        // 12-bit ADC
uint32_t Position=20;    // 32-bit fixed-point 0.001 cm
uint32_t OldPosition=20; 
uint32_t Height=156;
uint32_t OldHeight=156;
uint32_t OldPlaty;
int i=0;
int j=100;
int DrawFlag=0;
uint32_t PlatFlag=0; 
uint32_t Flag=0;
uint32_t platformy[50] = {4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 50, 48, 46, 44, 42, 40, 38, 36, 34, 32, 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4};
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
	
	void (*PeriodicTask)(void);
	
void Timer2_Init(void(*task)(void), unsigned long peri0d){
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  PeriodicTask = task;          // user function
  TIMER2_CTL_R = 0x00000000;    // 1) disable timer2A during setup
  TIMER2_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER2_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER2_TAILR_R = peri0d-1;    // 4) reload value
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution
  TIMER2_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER2_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN0_R = 1<<23;           // 9) enable IRQ 23 in NVIC
  TIMER2_CTL_R = 0x00000001;    // 10) enable timer2A
}

void Timer2A_Handler(void){
  TIMER2_ICR_R = TIMER_ICR_TATOCINT;// acknowledge TIMER2A timeout
  (*PeriodicTask)();                // execute user task
}

	
// **************Buttons_Init*********************
// Initialize piano key inputs, called once to initialize the digital ports
// Input: none 
// Output: none
void Buttons_Init(void){ 
	volatile int delay; 
	SYSCTL_RCGCGPIO_R |= 0x10; // enable clock for PORT E
	delay = 0; // let clock run 
	delay = 0;
	delay = 0;
	GPIO_PORTE_DIR_R &= ~0x03; // inputs on PORTE
	GPIO_PORTE_DEN_R |= 0x03; // 
}

// **************Buttons_In*********************
// Input from jump key inputs 
// Input: none 
// Output: 0 to 3 depending on keys
//   0x01 is just Key0, 0x02 is just Key1
//   bit n is set if key n is pressed
uint32_t Buttons_In(void){
	 uint32_t output = (GPIO_PORTE_DATA_R & 0x03); 
	return output; 
}

struct Sprite{
	uint32_t x,y;												//coordinates
	const unsigned short *image;			//ptr to image
	uint32_t w, h;											//size of image
	int32_t life;												//0 is dead, 1 is alive
};
typedef struct Sprite Sprite_t;
Sprite_t Firegirl;
Sprite_t platform;
Sprite_t Ledge1;
Sprite_t Ledge2;
Sprite_t Ledge3;
Sprite_t Bevo1;
Sprite_t Bevo2;
Sprite_t Bevo3;
Sprite_t Bevo4;
Sprite_t Bevo5;
Sprite_t Bevo6;
Sprite_t Bevo7;
Sprite_t Bevo8;
Sprite_t Bevo9;
Sprite_t Door1;
Sprite_t Door2;
Sprite_t Door3;
Sprite_t Ocean1;
Sprite_t Ocean2;
Sprite_t Ocean3;
Sprite_t Ocean4;
Sprite_t Ocean5;

void InitFire(void){
	Position = 20;
	Firegirl.x = Position;									//Initial position
	OldPosition = Position;
	Height = 156;
	Firegirl.y = Height;
	OldHeight = Height;
	Firegirl.image = firegirl;
	Firegirl.w = 5;
	Firegirl.h = 10;
	Firegirl.life = 1;
	DrawFlag = 1;
	platform.x = 96;
	platform.y = 160;
	platform.image = Platform;
	platform.w = 32;
	platform.h = 4;
	Bevo1.x = 60;
	Bevo1.y = 62;
	Bevo2.x = 50;
	Bevo2.y = 98;
	Bevo3.x = 45;
	Bevo3.y = 150;
	Bevo4.x = 60;
	Bevo4.y = 50;
	Bevo5.x = 4;
	Bevo5.y = 79;
	Bevo6.x = 60;
	Bevo6.y = 150;
	Bevo7.x = 60;
	Bevo7.y = 50;
	Bevo8.x = 60;
	Bevo8.y = 50;
	Bevo9.x = 60;
	Bevo9.y = 50;
	Door1.x = 118;
	Door1.y = 64;
	
	Door2.x = 118;
	Door3.x = 64;
	Ocean1.x = 30;
	Ocean1.y = 104;
	Ocean1.w = 16;
	Ocean2.x = 30;
	Ocean2.y = 155;
	Ocean2.w = 16;
//	Ocean3.x = 30;
//	Ocean3.y = 104;
//	Ocean3.w = 16;
//	Ocean4.x = 30;
//	Ocean4.y = 104;
//	Ocean4.w = 16;
//	Ocean5.x = 30;
//	Ocean5.y = 104;
//	Ocean5.w = 16;
}

void MovePlatform(void){
	if(i<50){
		OldPlaty = platform.y;
		platform.y=(160-platformy[i]);
		i++;
		if(Firegirl.x >= platform.x && Firegirl.y >= 104){
					PlatFlag=1;
					OldHeight = OldPlaty;
					Height = platform.y;
					Firegirl.x = Position;
					Firegirl.y = Height;
					ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
					ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
				}
		ST7735_DrawBitmap(platform.x, OldPlaty, ErasePlatform, 32, 4);
		ST7735_DrawBitmap(platform.x, platform.y, Platform, 32, 4);
		}
	if(i>=50){
		i=0;
	}
}
uint32_t Convert(uint32_t input){
	Position = ((0.0295*input)); 
  return(Position);
}

void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0; 						
	NVIC_ST_RELOAD_R = 75000; 
	NVIC_ST_CURRENT_R = 0; 
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000;
	NVIC_ST_CTRL_R = 0x07; 
}

void SysTick_Handler(void){
	if(Win == 1){
		ST7735_SetCursor(0,0);
			ST7735_FillScreen(0x0000);
			ST7735_SetCursor(0,80); 
			ST7735_OutString("Passed!Jewels:"); 
			ST7735_SetCursor(14,0) ; 
			LCD_OutDec(Score);  
		while(1){}
	}
	if(Firegirl.life == 0){
		ST7735_SetCursor(0,0);
			ST7735_FillScreen(0x0000);
		ST7735_SetCursor(0,80);
			ST7735_OutString("Failed!Jewels:"); 
			ST7735_SetCursor(14,0) ; 
			LCD_OutDec(Score); 
		while(1){}
	}
	button = Buttons_In(); 
	if(button != 0){
		if(button == lastbutton){
			PlayNothing();
			TIMER1_CTL_R = 0x00000001;
			OldHeight = Height;
			DrawFlag = 1;
		}else{
			lastbutton = button;
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayGrunt();
			OldHeight = Height;
			Height -= 40;
			Firegirl.y = Height;
			DrawFlag=1;
			Fall=1;
		}
		}else{
				PlayNothing();
				TIMER1_CTL_R = 0x00000001;
				OldHeight = Height;
				DrawFlag = 0;
		}
	OldPosition = Position;
	Data = ADC_In();
	Position = Convert(Data); 
	if(OldPosition != Position){
		if((PlatFlag==1 && Firegirl.x < platform.x) || (Firegirl.x >92 && Firegirl.y >104 && PlatFlag==0)){
			PlatFlag=0;
			DrawFlag=1;
			Fall=1;
		}
		Firegirl.x = Position;
		DrawFlag=1;
		Flag=1;
		}		
	if(DrawFlag == 1){
				ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
				ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
					if(Fall == 1){
						if(Firegirl.x >92 && Firegirl.y >= 104 && PlatFlag==0){
							while(Firegirl.y != 160){
							OldHeight = Height;
							Height += 1;
							Firegirl.y = Height;
							OldPosition = Position;
							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
							ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
								for(int i=0;i<50000;i++){
									i=i;
									i=i;
									i=i;
								}
							}
					Firegirl.life=0;
					Fall=0;
					lastbutton = 0;
						}
	else if(Firegirl.x <=92 && Firegirl.y > 113){
							while(Firegirl.y < 156){
							OldHeight = Height;
							Height += 1;
							Firegirl.y = Height;
							OldPosition = Position;
							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
							ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
								for(int i=0;i<50000;i++){
									i=i;
									i=i;
									i=i;
								}
							}
					Fall=0;
					lastbutton = 0;
						}
	else if(Firegirl.x >20 && Firegirl.x < 92 && Firegirl.y < 104 && Firegirl.y > 68){
							while(Firegirl.y != 104){
							OldHeight = Height;
							Height += 1;
							Firegirl.y = Height;
							OldPosition = Position;
							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
							ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
								for(int i=0;i<50000;i++){
									i=i;
									i=i;
									i=i;
								}
							}
					Fall=0;
					lastbutton = 0;
						}
	else if(Firegirl.x <20 && Firegirl.y < 104 && Firegirl.y > 68){
							while(Firegirl.y != 104){
							OldHeight = Height;
							Height += 1;
							Firegirl.y = Height;
							OldPosition = Position;
							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
							ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
								for(int i=0;i<50000;i++){
									i=i;
									i=i;
									i=i;
								}
							}
					Fall=0;
					lastbutton = 0;
						}
//	else if(Firegirl.x <=17 && Firegirl.y >= 10){
//							while(Firegirl.y != 85){
//							OldHeight = Height;
//							Height += 1;
//							Firegirl.y = Height;
//							OldPosition = Position;
//							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
//							ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
//								for(int i=0;i<50000;i++){
//									i=i;
//									i=i;
//									i=i;
//								}
//							}
//					Fall=0;
//					lastbutton = 0;
//						}
	else if(Firegirl.x >10 && Firegirl.y<=64){
							while(Firegirl.y != 64){
							OldHeight = Height;
							Height += 1;
							Firegirl.y = Height;
							OldPosition = Position;
							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
							ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
								for(int i=0;i<50000;i++){
									i=i;
									i=i;
									i=i;
								}
							}
					Fall=0;
					lastbutton = 0;
						}
//		else if(Firegirl.x >=1 && Firegirl.x <= 23 && Firegirl.y > 64){
//							while(Firegirl.y != 85){
//							OldHeight = Height;
//							Height += 1;
//							Firegirl.y = Height;
//							OldPosition = Position;
//							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
//							ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
//								for(int i=0;i<50000;i++){
//									i=i;
//									i=i;
//									i=i;
//								}
//							}
//					Fall=0;
//					lastbutton = 0;
//						}
//		else if(Firegirl.x >=10 && Firegirl.y < 85){
//							while(Firegirl.y != 64){
//							OldHeight = Height;
//							Height += 1;
//							Firegirl.y = Height;
//							OldPosition = Position;
//							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 5, 10);
//							ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
//								for(int i=0;i<50000;i++){
//									i=i;
//									i=i;
//									i=i;
//								}
//							}
//					Fall=0;
//					lastbutton = 0;
//						}
		if (Firegirl.y == 160 && PlatFlag == 0){
		Firegirl.life = 0;
		Fall=0;
					}
//					if(Firegirl.x){
//						while(Firegirl.y != 156 && Firegirl.y != ground.y){
//							OldHeight = Height;
//							Height += 4;
//							Firegirl.y = Height;
//							OldPosition = Position;
//							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 10, 20);
//							ST7735_DrawBitmap(Position, Height, firegirl, 10, 20);
//								for(int i=0;i<200000;i++){
//									i=i;
//									i=i;
//									i=i;
//								}
//							}
//					Fall=0;
//					lastbutton = 0;
//				}
//							
//						while(Firegirl.y != 156 && Firegirl.y != ground.y){
//							OldHeight = Height;
//							Height += 4;
//							Firegirl.y = Height;
//							OldPosition = Position;
//							ST7735_DrawBitmap(OldPosition, OldHeight, EraseSprite, 10, 20);
//							ST7735_DrawBitmap(Position, Height, firegirl, 10, 20);
//								for(int i=0;i<200000;i++){
//									i=i;
//									i=i;
//									i=i;
//								}
//							}
//					Fall=0;
//					lastbutton = 0;
//							
//							
				}
			}
	if (Firegirl.y == 160 && PlatFlag == 0){
		Firegirl.life = 0;
		Fall=0;
						}
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



int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
	ST7735_InitR(INITR_REDTAB);
	ST7735_FillScreen(0x0000);            // set screen to black
	InitFire();
	ST7735_DrawBitmap(4,160, Stage1Back, 128, 160);
	ST7735_DrawBitmap(4,113, Stage1Ledge1, 73, 9);
		ST7735_DrawBitmap(24,68, Stage1Ledge2, 118, 4);
		ST7735_DrawBitmap(110,64, door, 16, 22); 
		ST7735_DrawBitmap(60,62, TexasLogo, 14, 7);
		ST7735_DrawBitmap(50,98, TexasLogo, 14, 7);
		ST7735_DrawBitmap(45,150, TexasLogo, 14, 7);
	Stage1=1;
	PortFInit();
	ADC_Init();
	Sound_Init(); 						// initializes sound (& DAC as well) 
	Buttons_Init(); 
	SysTick_Init();
//  Output_Init();
//	ST7735_DrawBitmap(64, 80, Fire2, 10, 20);
	Timer1_Init(&PlayBackgroundMusic, A);  
	Timer2_Init(&MovePlatform, P);
	EnableInterrupts();
	while(1){
		while(Stage1){
		if(((Firegirl.x) == 45 && (Firegirl.y) == 156) && (Bevo1F == 0)){
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayTing();
			Score++;
			ST7735_DrawBitmap(45,150, EraseBevo, 14, 7);
			ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
			Bevo1F=1;
		}
		else if(((Firegirl.x) == Bevo2.x && (Firegirl.y) == 104) && (Bevo2F == 0)) {
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayTing();
			Score++;
			ST7735_DrawBitmap(50,98, EraseBevo, 14, 7);
			ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
			Bevo2F=1;
		}
		else if(((Firegirl.x) == Bevo1.x && (Firegirl.y) == 64) && (Bevo3F == 0)){
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayTing();
			Score++;
			ST7735_DrawBitmap(60,62, EraseBevo, 14, 7);
			ST7735_DrawBitmap(Position, Height, firegirl, 5, 10);
			Bevo3F=1;
		}
		else if(Firegirl.y == Door1.y && Firegirl.x == Door1.x){
			Win=1;
		}
		DrawFlag=0;
	}
	while(Stage2){
		if(Firegirl.x == Bevo4.x || Firegirl.y == Bevo4.y){
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayTing();
			Score++;
		}
		else if(Firegirl.x == Bevo5.x || Firegirl.y == Bevo5.y){
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayTing();
			Score++;
		}
		else if(Firegirl.x == Bevo6.x || Firegirl.y == Bevo6.y){
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayTing();
			Score++;
		}
		else if((Firegirl.y == Ocean1.y) && (Firegirl.x >= Ocean1.x && Firegirl.x <= (Ocean1.x + Ocean1.w))){
			Firegirl.life=0;
		}
		else if((Firegirl.y == Ocean2.y) && (Firegirl.x >= Ocean2.x && Firegirl.x <= (Ocean2.x + Ocean2.w))){
			Firegirl.life=0;
		}
		else if(Firegirl.x == Door2.x){
			Win=1;
		}
		DrawFlag=0;
	}
	
	while(Stage3){
		if(Firegirl.x == Bevo7.x || Firegirl.y == Bevo7.y){
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayTing();
			Score++;
		}
		else if(Firegirl.x == Bevo8.x || Firegirl.y == Bevo8.y){
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayTing();
			Score++;
		}
		else if(Firegirl.x == Bevo9.x || Firegirl.y == Bevo9.y){
			TIMER1_CTL_R = 0x00000000; 								// disable Timer1
			PlayTing();
			Score++;
		}
		else if((Firegirl.y == Ocean3.y) && (Firegirl.x >= Ocean3.x && Firegirl.x <= (Ocean3.x + Ocean3.w))){
			Firegirl.life=0;
		}
		else if((Firegirl.y == Ocean4.y) && (Firegirl.x >= Ocean4.x && Firegirl.x <= (Ocean4.x + Ocean4.w))){
			Firegirl.life=0;
		}
		else if((Firegirl.y == Ocean5.y) && (Firegirl.x >= Ocean5.x && Firegirl.x <= (Ocean5.x + Ocean5.w))){
			Firegirl.life=0;
		}
		else if(Firegirl.x == Door3.x){
			Win=1;
		}
		DrawFlag=0;
	}
	DrawFlag=0;
	if(Win == 1){
		ST7735_SetCursor(0,0);
			ST7735_FillScreen(0x0000);
			ST7735_SetCursor(0,80); 
			ST7735_OutString("Passed!Jewels:"); 
			ST7735_SetCursor(14,0) ; 
			LCD_OutDec(Score);  
		while(1){}
	}
	if(Firegirl.life == 0){
		ST7735_SetCursor(0,0);
			ST7735_FillScreen(0x0000);
		ST7735_SetCursor(0,80);
			ST7735_OutString("Failed!Jewels:"); 
			ST7735_SetCursor(14,0) ; 
			LCD_OutDec(Score); 
		while(1){}
	}
	}	
}



