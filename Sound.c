// Sound.c
// Runs on any computer
// Sound assets based off the original Space Invaders 
// Import these constants into your SpaceInvaders.c for sounds!
// Jonathan Valvano
// November 17, 2014
#include <stdint.h>
#include "Sound.h"
#include "../inc/tm4c123gh6pm.h"
#include "ST7735.h"
#include "DAC.h" 
#include "Timer0.h" 
#include "Timer1.h" 

#define	G (6378)
#define	D (8503)
#define A (5682) 

uint8_t sineindex = 0;
const unsigned char SineWave[32]={5,7,8,9,11,12,13,14,14,15,15,15,14,14,13,12,11,9,8,7,5,4,3,2,2,1,1,1,2,2,3,4}; 

void Sound_Init(void){
	Dac_Init(); 
	sineindex = 0; 
// write this
};

void PlaySineWave (void) {
	GPIO_PORTE_DATA_R ^= 0x08;
	sineindex = (sineindex +1) & 0x1F;
	DAC_Out(SineWave[sineindex]);
}

void PlayGrunt(void) {
	Timer0_Init((*PlaySineWave), G); 
}

void PlayTing (void) {
		Timer0_Init((*PlaySineWave), A);
}
//void Sound_Play(const uint8_t *pt, uint32_t count){
//// write this
//};
//void Sound_Shoot(void){
//// write this
//};
//void Sound_Killed(void){
//// write this
//};
//void Sound_Explosion(void){
//// write this
//};

//void Sound_Fastinvader1(void){
//// write this
//};
//void Sound_Fastinvader2(void){
//// write this
//};
//void Sound_Fastinvader3(void){
//// write this
//};
//void Sound_Fastinvader4(void){
//// write this
//};
//void Sound_Highpitch(void){
//// write this
//};
