/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../GLCD/GLCD.h" 
#include "../gestione_pong/gestione_pong.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/


//si utilizza una variabile per ogni elemento che si vuole controllare

volatile int key0=0; //controlla button0
volatile int key1=0;//controlla button1
volatile int key2=0;//controlla button2

volatile unsigned char play=0,pause=0,gameover=0;

void RIT_IRQHandler (void)
{			
	//BUTTON LAVORANO OGNI 100ms

	//controllo button0
	
	
	
	if(key0>1){
		if ((LPC_GPIO2->FIOPIN & (1<<10)) == 0 ){					
			if(key0==2){
				//key0 fa il reset					
				if(gameover){
					gameover=0;
					pong_init();
				}
			}
			key0++;
		}
		else{
			key0=0;
			//fatto perchè se premo int0 e lo rilascio prima di rientrare, il rit parte e non si ferma anche se dovrebbe fermarsi
			if(gameover){
				disable_RIT();
				reset_RIT();
			}
			NVIC_EnableIRQ(EINT0_IRQn);							 /* disable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
		}
	}
	else{
		if(key0==1)
			key0++;
	}
	
	//controllo button1
	if(key1>1){
		if ((LPC_GPIO2->FIOPIN & (1<<11)) == 0 ){
			if(key1==2){
			//key1 fa partire il gioco
				if(!play&&!gameover){
					write_text(0,Black);
					play=1;
				}
			}
			key1++;
		}
		else{
			key1=0;
			NVIC_EnableIRQ(EINT1_IRQn);							 /* disable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 0 pin selection */
		}
	}else{ 
		if(key1==1)
			key1++;
	}
	
	//controllo button2
	if(key2>1){
		if ((LPC_GPIO2->FIOPIN & (1<<12)) == 0 ){		
			if(key2==2){
				//key2 gestisce la pausa		
				if(play){
					if(!pause){
						pause=1;
						write_text(1,White);
						disable_RIT();
						reset_RIT();
						NVIC_EnableIRQ(EINT2_IRQn);							 /* disable Button interrupts			*/
						LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 0 pin selection */
					}
					else {
						pause=0;
						write_text(1,Black);
					}
				}
			}
			key2++;
		}else{
				key2=0;
				NVIC_EnableIRQ(EINT2_IRQn);							 /* disable Button interrupts			*/
				LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 0 pin selection */
				if(pause){
					disable_RIT();
					reset_RIT();
				}
		}
	}else{
		if(key2==1)
			key2++;
	}		
		//getisce posizione paddle
	if(!gameover && !pause)
		ADC_start_conversion();		

	if((!pause)){		
		if(!gameover&&play){
			gameover=gioco_pong(play);
			
			if(gameover){
				pause=0;
				play=0;
				disable_RIT();
				reset_RIT();
				NVIC_EnableIRQ(EINT0_IRQn);							 /* enable Button interrupts			*/
				LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
			}
		}
		else if(!gameover)
			gioco_pong(play); //così faccio aggiornamento barra senza muovere pallina, a gioco finito
	}	

	
	reset_RIT();

	LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
  return;
}



/******************************************************************************
**                            End Of File
******************************************************************************/
