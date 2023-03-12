/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           lib_timer.h
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        atomic functions to be used by higher sw levels
** Correlated files:    lib_timer.c, funct_timer.c, IRQ_timer.c
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "timer.h"

/******************************************************************************
** Function name:		enable_timer
**
** Descriptions:		Enable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void enable_timer( uint8_t timer_num )
{
  if ( timer_num == 0 )
  {
	LPC_TIM0->TCR = 1;
  }
  else if ( timer_num == 1 )
  {
	LPC_TIM1->TCR = 1;
  }else if ( timer_num == 2 )
  {
	LPC_TIM2->TCR = 1;
  }
  
	return;
 }

/******************************************************************************
** Function name:		disable_timer
**
** Descriptions:		Disable timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void disable_timer( uint8_t timer_num )
{
  if ( timer_num == 0 )
  {
	LPC_TIM0->TCR = 0;
  }else if ( timer_num == 1 )
  {
	LPC_TIM1->TCR = 0;
  }else if ( timer_num == 2 )
  {
	LPC_TIM1->TCR = 0;
  }
  
	return;
}

/******************************************************************************
** Function name:		reset_timer
**
** Descriptions:		Reset timer
**
** parameters:			timer number: 0 or 1
** Returned value:		None
**
******************************************************************************/
void reset_timer( uint8_t timer_num )
{
  uint32_t regVal;

  if ( timer_num == 0 )
  {
	regVal = LPC_TIM0->TCR;
	regVal |= 0x02;
	LPC_TIM0->TCR = regVal;
  }
  else if ( timer_num == 1 )
  {
	
	regVal = LPC_TIM1->TCR;
	regVal |= 0x02;
	LPC_TIM1->TCR = regVal;
	}else if ( timer_num == 2 )
  {
	
	regVal = LPC_TIM2->TCR;
	regVal |= 0x02;
	LPC_TIM2->TCR = regVal;
  
	
	}
  return;
}

uint32_t init_timer ( uint8_t timer_num, uint8_t MatchReg, uint8_t SRImatchReg, uint32_t Prescaler, uint32_t TimerInterval )
{
	//cosa vogliono dire le variabili
	//timer_num:  scelgo quale timer inizializzare
	//MatchReg: scelgo quale match register inizializzare
	//SRImatchReg: scelgo i valori da dare al match register scelto (configurando i 3 bit famosi)
	//Prescaler: configura il prescaler
	//TimerInterval: l'intervallo con cui deve lavorare il timer
	
	
	//timer di default con priorità: timer0: 4 ,timer1: 5 ,timer2: 0 
	
  if ( timer_num == 0 ){ //fare questo se il timer è 0
		LPC_TIM0->PR = Prescaler;
		if(MatchReg==0){
			LPC_TIM0->MR0 = TimerInterval;
			LPC_TIM0->MCR |= SRImatchReg <<3*MatchReg;	//dice come si devono comportare i match register di timer 	0. //x es con SRImatchReg=3 e MatchReg=0 abiliti le funzioni di MatchReg 0 
																									//NB SE HO MatchReg=1 BISOGNA SHIFTARE A SX DI 3 POSIZIONI PERCHè MCR GESTISCE NEI 3 BIT MENO SIGNIFICATIVI MatchReg0 , 
																									//POI NEI 3 BIT SUCCESSIVI MatchReg1 E COSì VIA
																									//STA OPERAZIONE SI FA IN OR PERCHè IO ABILITERò UN MATCH REGISTER PER VOLTA. 
																									//PER ABILITARE PIù MATCH REGISTER UNO X VOLTA SENZA PERDERE I VALORI PASSATI DI MCR SI DEVE USARE LA OR
		}
		else if(MatchReg==1){
			LPC_TIM0->MR1 = TimerInterval;
			LPC_TIM0->MCR |= SRImatchReg <<3*MatchReg;
		}
		else if(MatchReg==2){
			LPC_TIM0->MR2 = TimerInterval;
			LPC_TIM0->MCR |= SRImatchReg <<3*MatchReg;
		}
		else if(MatchReg==3){
			LPC_TIM0->MR3 = TimerInterval;
			LPC_TIM0->MCR |= SRImatchReg <<3*MatchReg;
		}

		
		NVIC_EnableIRQ(TIMER0_IRQn); //abilità la possibilità di avere interrupt per timer0	//quando scatterà il timer, si eseguirà quello che sta in TIMER0_IRQHandler
	//	NVIC_SetPriority(TIMER0_IRQn, 4);		/* less priority than buttons */
		return (1);
  }
	
	else if ( timer_num == 1 ){ //fare questo se timer è timer 1 //stesse cose di prima , però per timer1
		LPC_TIM1->PR = Prescaler;		
		if(MatchReg==0){
			LPC_TIM1->MR0 = TimerInterval; 
			LPC_TIM1->MCR |= SRImatchReg <<3*MatchReg;
		}
		else if(MatchReg==1){
			LPC_TIM1->MR1 = TimerInterval;
			LPC_TIM1->MCR |= SRImatchReg <<3*MatchReg;
		}
		else if(MatchReg==2){
			LPC_TIM1->MR2 = TimerInterval;
			LPC_TIM1->MCR |= SRImatchReg <<3*MatchReg;
		}
		else if(MatchReg==3){
			LPC_TIM1->MR3 = TimerInterval;
			LPC_TIM1->MCR |= SRImatchReg <<3*MatchReg;
		}
		NVIC_EnableIRQ(TIMER1_IRQn); 
		return (1);
	}
	
	else if ( timer_num == 2 ){ //fare questo se timer è timer 2	//stesse cose di prima , però per timer2
		LPC_TIM2->PR = Prescaler;
		if(MatchReg==0){
			LPC_TIM2->MR0 = TimerInterval; 
			LPC_TIM2->MCR |= SRImatchReg <<3*MatchReg;
		}
		else if(MatchReg==1){
			LPC_TIM2->MR1 = TimerInterval;
			LPC_TIM2->MCR |= SRImatchReg <<3*MatchReg;
		}
		else if(MatchReg==2){
			LPC_TIM2->MR2 = TimerInterval;
			LPC_TIM2->MCR |= SRImatchReg <<3*MatchReg;
		}
		else if(MatchReg==3){
			LPC_TIM2->MR3 = TimerInterval;
			LPC_TIM2->MCR |= SRImatchReg <<3*MatchReg;
		}
		NVIC_EnableIRQ(TIMER2_IRQn); 
//		NVIC_SetPriority(TIMER2_IRQn, 0);
		return (1);
	}
  return (0);
}

/******************************************************************************
**                            End Of File
******************************************************************************/
