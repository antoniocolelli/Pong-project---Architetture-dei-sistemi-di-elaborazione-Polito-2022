#include "lpc17xx.h"
#include "dac.h"
#include "../timer/timer.h"

/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void DAC_init (void) {
	LPC_PINCON->PINSEL1 |= (1<<21);
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);
	


}

void suona(uint32_t frequenza){
	
	DAC_start_conversion(frequenza);

}



void DAC_start_conversion (uint32_t frequenza) {  //funziona con timer0

	disable_timer(0);
	reset_timer(0);
	init_timer(0,0,3,1,frequenza);
	enable_timer(0);

}				 
