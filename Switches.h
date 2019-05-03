// Switches.h
// This software configures the jump keys
// Program written by: Deepanshi Sharma and Danica Corbita
// Date Created: 5/2/2019 
// Last Modified: 5/2/2019
// Hardware connections

#ifndef SWITCHES_H
#define SWITCHES_H
#include <stdint.h>
// Header files contain the prototypes for public functions
// this file explains what the module does

// **************Switches_Init*********************
// Initialize jump key inputs, called once to initialize the digital ports
// Input: none 
// Output: none
void Switches_Init(void);

// **************Switches_In*********************
// Input from jump key inputs 
// Input: none 
// Output: 0 to 7 depending on keys
//   0x01 is just Key0, 0x02 is just Key1, 0x04 is just Key2
//   bit n is set if key n is pressed
uint32_t Switches_In(void);

#endif

