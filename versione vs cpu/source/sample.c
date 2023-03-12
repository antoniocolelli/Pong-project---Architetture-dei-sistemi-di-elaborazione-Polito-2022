/*----------------------------------------------------------------------------
 * Name:    sample.c
 * Purpose: 
 *		to control led11 and led 10 through EINT buttons (similarly to project 03_)
 *		to control leds9 to led4 by the timer handler (1 second - circular cycling)
 * Note(s): this version supports the LANDTIGER Emulator
 * Author: 	Paolo BERNARDI - PoliTO - last modified 15/12/2020
 *----------------------------------------------------------------------------
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2017 Politecnico di Torino. All rights reserved.
 *----------------------------------------------------------------------------*/

                  
#include <stdio.h>
#include "LPC17xx.H"                    /* LPC17xx definitions                */
#include "led/led.h"
#include "button_EXINT/button.h"
#include "timer/timer.h"
#include "ADC/adc.h"
#include "RIT/RIT.h"
#include "GLCD/GLCD.h" 
#include "DAC/dac.h"
#include "gestione_pong/gestione_pong.h"
	
/* Led external variables from funct_led */
#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif
/*----------------------------------------------------------------------------
  Main Program
 *------------0----------------------------------------------------------------*/
int main (void) {

/////////////////////////////////////////////////////////////////////////////////////
// NEL MIO CASO CON CUSTOM SCALE FACTOR A 20 NON SI BLOCCA MAI, SOPRA 20 SI BLOCCA //
/////////////////////////////////////////////////////////////////////////////////////

 	SystemInit();  																																				/* System Initialization (i.e., PLL)  */
	BUTTON_init();																																				/* BUTTON Initialization              */
	ADC_init();																																						/* ADC Initialization									*/
 	LCD_Initialization();
  LCD_Clear(Black,Red); //modificata per fare i wall	
	pong_init();
	DAC_init();
	init_RIT(0x004c4b40);																																	/* RIT Initialization 50 msec       */	
	enable_RIT(); 
	
	LPC_SC->PCON |= 0x1;									/* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);						
		
	LPC_PINCON->PINSEL1 |= (1<<21);
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);

  while (1) {                           /* Loop forever                       */	

		__ASM("wfi");		
  }

}
