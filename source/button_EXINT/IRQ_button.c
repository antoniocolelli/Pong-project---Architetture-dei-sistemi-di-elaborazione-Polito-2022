#include "button.h"
#include "lpc17xx.h"

#include "../led/led.h"
#include "../RIT/RIT.h"		  			/* you now need RIT library 			 */



//dichiarate nel rit. Uso una variabile per controllare un bottone
extern int key0; //controlla button0
extern int key1;//controlla button1
extern int key2;//controlla button2

void EINT0_IRQHandler (void)	  
{
	NVIC_DisableIRQ(EINT0_IRQn);	//DISABILITO INTERRUPT PER PRECAUZIONE VISTO CHE ORA SI LAVORA CON IL RIP NELLA PRESSIONE DEL PULSANTE	/* disable Button interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 20);     /* GPIO pin selection */ 
	key0=1;
	LPC_SC->EXTINT &= (1 << 0);     /* clear pending interrupt         */
}

void EINT1_IRQHandler (void)	  
{
	NVIC_DisableIRQ(EINT1_IRQn);		/* disable Button interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 22);     /* GPIO pin selection */ 
	key1=1;
	LPC_SC->EXTINT &= (1 << 1);     /* clear pending interrupt         */
}


void EINT2_IRQHandler (void)	  
{
	NVIC_DisableIRQ(EINT2_IRQn);		/* disable Button interrupts			 */
	LPC_PINCON->PINSEL4    &= ~(1 << 24);     /* GPIO pin selection */ 
	key2=1;
	LPC_SC->EXTINT &= (1 << 2);     /* clear pending interrupt         */    
}
