/*----------------------------------------------------------------------------
  Include
	*----------------------------------------------------------------------------*/
#include "lpc17xx.h"
#include "../GLCD/GLCD.h" 
#include "gestione_pong.h"
#include "../DAC/dac.h"

/*----------------------------------------------------------------------------
  External variables
	*----------------------------------------------------------------------------*/

extern unsigned short AD_current;   
/*----------------------------------------------------------------------------
  Internal variables
	*----------------------------------------------------------------------------*/

//coordinate pallina. 
//non possono essere unsigned anche se il display parte da 0 perchè quando nel bordo sx, x potrebbe prendere un valore negativo se decidessi di muovere pallina di un determinato numero di pixel per volta . stessa cosa per y

int x=MAX_X-1  -5-2;
int y=MAX_Y/2;

unsigned int centro_paddle=0;

int spin=0;//lo spin va messo globale perchè serve azzerarlo in init //0= no spin, 1,2,3 = spin ma fase quadrato, 4,5,6=spin fase rombo
volatile int punteggio_current,punteggio_max=100;

/*----------------------------------------------------------------------------
  Function that manages the pong
	*----------------------------------------------------------------------------*/

unsigned int quante_cifre(int valore){

	unsigned int n_cifre=0;	
	do{
		n_cifre++;
		valore/=10;
	}while(valore>=1);

	return n_cifre;
}



void pong_init(void){
	volatile int i;
	
	//RESETTA LO SPIN
	spin=0;
	
	//STAMPA PUNTEGGI
	write_text(3,Black); //pulizia da eventuale precedente partita
	punteggio_current=0; //inizializzazione punteggio corrente
	write_text(3,White); //stampa valori iniziali per nuova partita

	//non mi trovo nella prima inizializzazione,
	//devo pulire i pixel della pallina "morta" e della scritta di lose
	if(centro_paddle!=0){
		for(i=0;i<5;i++)
			LCD_DrawLine( x-2,y-2+i ,x+2,y-2+i,Black);	
		write_text(2,Black);
	}
	
	//STAMPA TESTO INIZIO GIOCO
	write_text(0,White);
	
	//mi trovo nella prima inizializzazione, 
	//devo stampare a video il paddle per la prima volta
	if(centro_paddle==0){
		
		//paddle a inizio gico è stampato al centro. Poi andrà verso la direzione decisa dal potenziometro
		centro_paddle=MAX_X/2;
		
		// la posizione del pong viene settata solo alla prima inizializzazione, al centro, 
		// perchè nel gioco del pong se si subisce gol, la pallina non riparte dal centro 
		// ma il paddle resta dove stava prima. è accortezza del giocatore spostarlo
		for(i=0;i<spessore_paddle;i++)
			LCD_DrawLine( centro_paddle-dim_base/2,y_paddle-i,  centro_paddle+dim_base/2, y_paddle-i , Green ); //la linea va da centro - dim_linea/2 a centro + dim_linea/2
	}

	//POSIZIONE INIZIALE PALLINA
	x=MAX_X-1-5-2; 
	y=MAX_Y/2;
		//ACCORTEZZE X
			//-1 perchè x va da 0 a MAX_X-1 ; 
			//-5 per non sovrascrivere il muro ; 
			//-2 perchè x rappresenta il centro della pallina, che poi si estende di 2 pixel a sx e 2 a dx
	
	//STAMPO A SCHERMO PALLINA IN POSIZIONE INIZIALE
	for(i=0;i<5;i++){
		LCD_DrawLine( x-2,y-2+i ,x+2,y-2+i,Yellow);
	}
	
	return;
}

void write_text(int situazione,uint16_t Color){
	
	//SITUAZIONE: 0: SITUAZIONE INIZIALE PRIMA DI PREMERE START
	//SITUAZIONE: 1: SITUAZIONE GIOCO IN PAUSA
	//SITUAZIONE: 2: SITUAZIONE LOSE
	//SITUAZIONE: 3: SITUAZIONE STAMPA PUNTEGGI

	char punteggio_in_char[5] = ""; //max 99999	
	
	switch(situazione){
		case 0:
			GUI_Text(10,20, (uint8_t *) "LET'S PLAY",Color,Black);
			GUI_Text(10,60, (uint8_t *) "MOVE THE PADDLE TO DESIRED",Color,Black);
			GUI_Text(10,60+16, (uint8_t *) "INITIAL POSITION",Color,Black);
			GUI_Text(10,116, (uint8_t *) "PRESS KEY1 TO START",Color,Black);
		break;
		case 1:
			GUI_Text(10,20, (uint8_t *) "PRESS KEY2 TO RESUME",Color,Black);
		break;		
		case 2:
			GUI_Text(10,20, (uint8_t *) "YOU LOSE",Color,Black);
			GUI_Text(10,60, (uint8_t *) "PRESS INT0 TO RESET",Color,Black);
		break;
		case 3: //punteggio e record
			if(punteggio_current<10)
				sprintf(punteggio_in_char,"%1d",punteggio_current);
			else if(punteggio_current<100)
				sprintf(punteggio_in_char,"%2d",punteggio_current);
			else if(punteggio_current<1000)
				sprintf(punteggio_in_char,"%3d",punteggio_current);
			else if(punteggio_current<10000)
				sprintf(punteggio_in_char,"%4d",punteggio_current);
			else
				sprintf(punteggio_in_char,"%5d",punteggio_current);
			
			GUI_Text(10, 152, (uint8_t *) punteggio_in_char, Color, Black); 
			
			if(punteggio_current==punteggio_max){
				 GUI_Text(227 - 8*quante_cifre(punteggio_current), 10, (uint8_t *) punteggio_in_char, Color, Black);
			}else{
				if(punteggio_max<10)
					sprintf(punteggio_in_char,"%1d",punteggio_max);
				else if(punteggio_max<100)
					sprintf(punteggio_in_char,"%2d",punteggio_max);
				else if(punteggio_max<1000)
					sprintf(punteggio_in_char,"%3d",punteggio_max);
				else if(punteggio_max<10000)
					sprintf(punteggio_in_char,"%4d",punteggio_max);
				else
					sprintf(punteggio_in_char,"%5d",punteggio_max);
			
				GUI_Text(227 - 8*quante_cifre(punteggio_max), 10, (uint8_t *) punteggio_in_char, Color, Black);
			}
		break;		
		default:
		break;
		}
}


int calcolo_centro(unsigned short val_potenziometro){
	
	//val_potenziometro è compreso tra 0 e 4095. Lo trasformo in un valore tra 0 e 200 e poi faccio +20
	int i;
	
	i=val_potenziometro*(200)/0xFFF;
	i+=20;

	//ora ho un valore tra 20 e 220 . Questo è il range di valori in cui può trovarsi il centro della barra 

	return i;

}




int gioco_pong(int in_game){


  volatile unsigned int diff;
  static unsigned int i=0; //gestisce i cicli della scrittura delle colonne	
	static unsigned int centro_curr,centro_richiesto; //variabili di lavoro del paddle
	static unsigned char arrivato=0; //mi indica se centro richiesto è stato raggiunto
	volatile int cicli_red; //per i cicli di scrittura per creare un rate della pallina costante
	volatile int info_return=0;
	
	centro_richiesto=calcolo_centro(AD_current);
	centro_curr=centro_paddle;
	
	if(centro_richiesto>centro_curr)
		diff=centro_richiesto-centro_curr;
	else
		diff=centro_curr-centro_richiesto;
  
	
	//con emulatore 5 è un valore che tiene più stabile la posizione a discapito di micro cambiamenti
	//arrivato ha compito di smistare tra la poca differenza tra centro_richiesto e centro_corrente dovuta 
	//alle non idealità del paddle con la poca differenza dovuta al fatto che il paddle, dopo il movimento, deve ancora arrivare alla corretta posizione
	//centro_richiesto è appunto una richiesta. Si tiene in considerazione solo il centro corrente
	//da qui codice

	
	
	if(diff>5 || !arrivato){  		
		//AGGIORNAMENTI VERTICALI
		for(i=0;i<colonne_spostate_per_rit && centro_richiesto!=centro_curr;i++){
		
			if(centro_richiesto>centro_curr){
				LCD_DrawLine( centro_curr-dim_base/2,y_paddle-spessore_paddle+1,  centro_curr-dim_base/2, y_paddle , Black );
				centro_curr++;
				LCD_DrawLine( centro_curr+dim_base/2,y_paddle-spessore_paddle+1,  centro_curr+dim_base/2, y_paddle , Green ); //la linea va da centro - dim_linea/2 a centro + dim_linea/2
			}
			else if(centro_richiesto<centro_curr){
				LCD_DrawLine( centro_curr+dim_base/2,y_paddle-spessore_paddle+1,  centro_curr+dim_base/2, y_paddle , Black );
				centro_curr--;
				LCD_DrawLine( centro_curr-dim_base/2,y_paddle-spessore_paddle+1,  centro_curr-dim_base/2, y_paddle , Green ); //la linea va da centro - dim_linea/2 a centro + dim_linea/2
			}
		
		}
		if(centro_richiesto==centro_curr)
			arrivato=1;
		else 
			arrivato=0;

		centro_paddle=centro_curr;

	}else{   
		//fatto per tenere sempre lo stesso framerate della pallina 
		//anche quando il paddle si muove (altrimenti andrebbe un pelo più veloce)
		if(in_game){
			//scritture random per mantenere sempre lo stesso rate della pallina
			for(i=colonne_spostate_per_rit;i>0;i--){
				//tolgo e rimetto sempre allo stesso punto per mantenere le velocità
				LCD_DrawLine( centro_curr+dim_base/2,y_paddle-spessore_paddle+1,  centro_curr+dim_base/2, y_paddle , Green );
				LCD_DrawLine( centro_curr+dim_base/2,y_paddle-spessore_paddle+1,  centro_curr+dim_base/2, y_paddle , Green ); 
			}	
		}
	}
		
	if(in_game){ 
		info_return=genera_pallina();
		//0=mi sto muovendo nello spazio, nulla di rilevante				
		//1=morto
		//2=tocco pallina
		if(info_return==2){ 
			//aggiorna il punteggio
			if(punteggio_current>100)
				punteggio_current+=10;
			else
				punteggio_current+=5;
			if(punteggio_current>99999){ //caso overflow. Ho deciso di far arrivare il punteggio a 99999
				write_text(3,Black);
				punteggio_current=0;
				punteggio_max=99999;
			}
				
			if(punteggio_current>punteggio_max)
				punteggio_max=punteggio_current;
	
			//stampo a schermo i punteggi
			write_text(3,White);
		
			info_return=0;
	
		}else if(info_return==1){
			
			//stampo scritta lose
			write_text(2,White);
		}
	}
		
	return info_return; //ritorno 0 se è tutto ok e sto continuando a giocare, 1 se ho perso
}

//genera_pallina ritorna info sul gioco. 
//ritorna 0 se non succede nulla, 
//1 se la pallina è caduta, 
//2 se è necessario aggiornare il valore di punteggio corrente perchè è cambiato 

int genera_pallina(void){ 

	volatile unsigned int info_return=0;
	
	unsigned int i;
	
	static int go_down=1,go_right=1;
	static unsigned salto_x=max_salto_pixel;
	static unsigned salto_y=max_salto_pixel;
	
	volatile int sx_paddle,dx_paddle;
	static unsigned int angolo;
	
	if(  ! ( (  x>=10 && x<=(10+8*quante_cifre(punteggio_current))  ) &&  (y>=152 && y<=168) )      &&
			 ! ( (  x>=(227-8*quante_cifre(punteggio_max)) && x<=227    ) &&  (y>=10  && y<=26 ) )
		)
	{
		
		if(!spin || spin==2|| spin==3|| spin==4){
			for(i=0;i<5;i++){
				LCD_DrawLine( x-2,y-2+i ,x+2,y-2+i, Black);
			}
		}else{
			LCD_DrawLine( x,y-2 ,x,y-2, Black);
			LCD_DrawLine( x-1,y-1 ,x+1,y-1, Black);
			LCD_DrawLine( x-2,y ,x+2,y, Black);
			LCD_DrawLine( x-1,y+1 ,x+1,y+1, Black);
			LCD_DrawLine( x,y+2 ,x,y+2, Black);
		}
	}else{
		for(i=0;i<5;i++){
			LCD_DrawLine( 0,0 ,5,0, Red); //scrittura random per 
		}
	}

	
	if(y==275){ 
		sx_paddle=centro_paddle-dim_base/2;
		dx_paddle=centro_paddle+dim_base/2;

		
		if( ( (x-2)<dx_paddle && (x+2)>dx_paddle ) || ( (x+2)<=dx_paddle && (x-2)>=sx_paddle ) || ( (x-2)<sx_paddle && (x+2)>sx_paddle )   ){ //se tocco il paddle
			info_return=2;
			suona(2120); //262Hz

		//3 casi.

			if( (x-2)<dx_paddle && (x+2)>dx_paddle ){	//caso estremo destro
				salto_x=max_salto_pixel+1;
				salto_y=1;		
				go_right=1;
			}else if( (x-2)<sx_paddle && (x+2)>sx_paddle ){ //caso estremo sinistro
				salto_x=max_salto_pixel+1;
				salto_y=1;	
				go_right=-1;
			}else{  //caso tocco interno
				if(x>centro_paddle){
					i=x-centro_paddle;
					angolo=90*i/(dim_base/2);
					if(angolo>45){
						salto_x=max_salto_pixel;
						salto_y=max_salto_pixel*(-angolo+90)/45; //x mantenere sempre un valore 0<=val<=45
						if(salto_y==0)
							salto_y=1;
					}else{
						salto_y=max_salto_pixel;
						salto_x=max_salto_pixel*angolo/45;	
					}
					go_right=1;
				}
				else{ //if(x<=centro_paddle){
					i=centro_paddle-x;
					angolo=90*i/(dim_base/2);
					if(angolo>45){
						salto_x=max_salto_pixel;
						salto_y=max_salto_pixel*(-angolo+90)/45; //x mantenere sempre un valore 0<=val<=45
						if(salto_y==0)
							salto_y=1;
					}else{
						salto_y=max_salto_pixel;
						salto_x=max_salto_pixel*angolo/45;	
					}
					go_right=-1;
				}
			}

			//casi in cui spinnare 	
			if(salto_x>=max_salto_pixel*3/4 && salto_y<=max_salto_pixel*1/3 && spin==0)
				spin=1;
			else if (! (salto_x>=max_salto_pixel*3/4 && salto_y<=max_salto_pixel*1/3) )
				spin=0;
			
			go_down=-1; //per andare su
		}
		else{
			info_return=1;
			 //scendo un po' giù e stampo la pallina lì
			for(i=0;i<5;i++)
				LCD_DrawLine( x-2,y-2+i ,x+2,y-2+i,Yellow);
			
			for(i=0;i<20;i++){
				LCD_DrawLine( x-2,y-2 ,x+2,y-2, Black);
				y++;
				LCD_DrawLine( x-2,y+2 ,x+2,y+2,Yellow);			
			}
			//ora prepara i nuovi valori di diagonale
			salto_x=max_salto_pixel;
			salto_y=max_salto_pixel;
			return info_return;
		}

	}

	else if(y==7){
		go_down=1;
		suona(3175); //175Hz

	}
	
	if(x==7){
		go_right=1;
		suona(3175); //175Hz
		

	}

	else if(x==232){
		go_right=-1;
		suona(3175); //175Hz
		

	}	
	if(salto_y==1){ //se saltoy è 1 sono troppo lento. Raddoppia le velocità
		salto_y*=2;
		salto_x*=2;
	}
	
	y+=salto_y*go_down; 
	x+=salto_x*go_right; 
	
	
	if(x<=7)//(ricordo che asse x parte da 0) 7 è IL VALORE CHE HA IL CENTRO DELLA PALLINA, ASSE X, PER IL TOCCO DEL MURO SX
		x=7;
	else if(x>=232)//(ricordo che asse x termina a 239) 232 è IL VALORE CHE HA IL CENTRO DELLA PALLINA, ASSE X, PER IL TOCCO DEL MURO DX
		x=232;
		
	if(y<=7)//(ricordo che asse y parte da 0) 7 è IL VALORE CHE HA IL CENTRO DELLA PALLINA, ASSE Y, PER IL TOCCO DEL TETTO
		y=7;
	else if(y>=275) //(ricordo che asse y termina a 319) 275 è IL VALORE CHE HA IL CENTRO DELLA PALLINA, ASSE Y, PER IL TOCCO DEL PADDLE
		y=275;


	if(  ! ( (  x>=10 && x<=(10+8*quante_cifre(punteggio_current))  ) &&  (y>=152 && y<=168) )      &&
			 ! ( (  x>=(227-8*quante_cifre(punteggio_max)) && x<=227    ) &&  (y>=10  && y<=26 ) )
		){
		if(!spin || spin<=3){
			for(i=0;i<5;i++){
				LCD_DrawLine( x-2,y-2+i ,x+2,y-2+i, Yellow);
			}
			if(spin>0)
				spin++;
		}else{		
			LCD_DrawLine( x,y-2 ,x,y-2, Yellow);
			LCD_DrawLine( x-1,y-1 ,x+1,y-1, Yellow);
			LCD_DrawLine( x-2,y ,x+2,y, Yellow);
			LCD_DrawLine( x-1,y+1 ,x+1,y+1, Yellow);
			LCD_DrawLine( x,y+2 ,x,y+2, Yellow);
			
			if(spin>=4){
				if(spin==6)
					spin=1;
				else
					spin++;
			}	
		}
	}else{ //se sto nel confine non scrivibile
		for(i=0;i<10;i++){ //10 perchè 5 cancello il precedente e 5 scrivo il corrente 
			LCD_DrawLine( 0,0 ,5,0, Red); //scrittura random per mantenere sempre stessa velocità
		}
	}
	
	return info_return;
}
