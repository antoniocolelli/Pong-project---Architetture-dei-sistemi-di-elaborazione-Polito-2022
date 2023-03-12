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

unsigned int centro_paddle_user=0;
unsigned int centro_paddle_cpu=0;

int spin=0;//lo spin va messo globale perchè serve azzerarlo in init //0= no spin, 1,2,3 = spin ma fase quadrato, 4,5,6=spin fase rombo
volatile int punteggio_current_user,punteggio_current_cpu,punteggio_max=100;

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
		//pulizia da eventuale precedente partita
	if(centro_paddle_user!=0){	
		write_text(2,Black);
		write_text(3,Black); 
		write_text(4,Black);
	}
	
	punteggio_current_user=0; //inizializzazione punteggio corrente user
	punteggio_current_cpu=0; //inizializzazione punteggio corrente cpu
	
	write_text(3,White); //stampa valori iniziali per nuova partita
	write_text(4,White);
		
	//STAMPA TESTO INIZIO GIOCO
	write_text(0,White);
	
	//mi trovo nella prima inizializzazione, 
	//devo stampare a video il paddle per la prima volta
	if(centro_paddle_user==0){
		
		//paddle a inizio gico è stampato al centro. Poi andrà verso la direzione decisa dal potenziometro
		centro_paddle_user=MAX_X/2;
		centro_paddle_cpu=MAX_X/2;

		// la posizione del pong viene settata solo alla prima inizializzazione, al centro, 
		// perchè nel gioco del pong se si subisce gol, la pallina non riparte dal centro 
		// ma il paddle resta dove stava prima. è accortezza del giocatore spostarlo
		for(i=0;i<spessore_paddle;i++){
			LCD_DrawLine( centro_paddle_user-dim_base/2,y_paddle_user-i,  centro_paddle_user+dim_base/2, y_paddle_user-i , Green ); //la linea va da centro - dim_linea/2 a centro + dim_linea/2
			LCD_DrawLine( centro_paddle_cpu-dim_base/2,y_paddle_cpu+i,  centro_paddle_cpu+dim_base/2, y_paddle_cpu+i , Green ); //la linea va da centro - dim_linea/2 a centro + dim_linea/2
		}
	}

	
	init_pallina();
	
	return;
}

void write_text(int situazione,uint16_t Color){
	
	//SITUAZIONE: 0: SITUAZIONE INIZIALE PRIMA DI PREMERE START
	//SITUAZIONE: 1: SITUAZIONE GIOCO IN PAUSA
	//SITUAZIONE: 2: SITUAZIONE WIN/LOSE
	//SITUAZIONE: 3: SITUAZIONE STAMPA PUNTEGGIO USER
	//SITUAZIONE: 4: SITUAZIONE STAMPA PUNTEGGIO CPU

	//GUI_Text e PutChar modificate per permettere scrittura al contrario. L'ultimo parametro di GUI_Text è 0 se si stampa in direzione USER, 1 se si stampa in direzione CPU
	char punteggio_in_char[5] = ""; 
	switch(situazione){
		case 0:
			GUI_Text(10,258-16*5, (uint8_t *) "LET'S PLAY",Color,Black,0);
			GUI_Text(10,258-16*3, (uint8_t *) "MOVE THE PADDLE TO DESIRED",Color,Black,0);
			GUI_Text(10,258-16*2, (uint8_t *) "INITIAL POSITION",Color,Black,0);
			GUI_Text(10,258, (uint8_t *) "PRESS KEY1 TO START",Color,Black,0);
		break;
		case 1:
			GUI_Text(10,226, (uint8_t *) "PRESS KEY2 TO RESUME",Color,Black,0);
		break;		
		case 2:
			if(punteggio_current_user==5){
				GUI_Text(10,226-16, (uint8_t *) "YOU WIN",Color,Black,0);
				GUI_Text(227-8*strlen("YOU LOSE")+8,125-16, (uint8_t *) "YOU LOSE",Color,Black,1);
			}else if (punteggio_current_cpu==5){
				GUI_Text(10,226-16, (uint8_t *) "YOU LOSE",Color,Black,0);
				GUI_Text(227-8*strlen("YOU WIN")+8,125-16, (uint8_t *) "YOU WIN",Color,Black,1);
			}
			GUI_Text(10,226, (uint8_t *) "PRESS INT0 TO RESET",Color,Black,0);
		break;	
		case 3: //punteggio USER

			sprintf(punteggio_in_char,"%1d",punteggio_current_user);
			GUI_Text(10, 152, (uint8_t *) punteggio_in_char, Color, Black,0); 
			
		break;		
		case 4: //punteggio CPU
			sprintf(punteggio_in_char,"%1d",punteggio_current_cpu);
			GUI_Text(227 - 8*quante_cifre(punteggio_current_cpu)+8, 152+16, (uint8_t *) punteggio_in_char, Color, Black,1); //+8,-16 PERCHè GIRANDO AL CONTRARIO SI GIRA AL CONTRARIO ANCHE IL PUNTO DI INIZIO
			
		break;		
		default:
		break;
	}
}


int calcolo_centro(unsigned short val_potenziometro){
	
	//val_potenziometro è compreso tra 0 e 4095. Lo trasformo in un valore tra 0 e 189 e poi faccio +25
	
//	|MURO|               |MURO|
//	0		 4              235  239
//        5           234
//C.PADL.    25    214
	
	int i;
	i=val_potenziometro*(189)/0xFFF;
	i+=25;
	return i;

}




int gioco_pong(int in_game){

  volatile unsigned int diff;
  static unsigned int i=0; //gestisce i cicli della scrittura delle colonne	
	static unsigned int centro_curr_user,centro_richiesto_user; //variabili di lavoro del paddle
	static unsigned int centro_curr_cpu,go_right_paddle_cpu=1;
	static unsigned char arrivato=0; //mi indica se centro richiesto è stato raggiunto
	
	volatile int cicli_red; //per i cicli di scrittura per creare un rate della pallina costante
	volatile int info_return=0;
	
	centro_richiesto_user=calcolo_centro(AD_current);
	centro_curr_user=centro_paddle_user;
	centro_curr_cpu=centro_paddle_cpu;
	if(centro_richiesto_user>centro_curr_user)
		diff=centro_richiesto_user-centro_curr_user;
	else
		diff=centro_curr_user-centro_richiesto_user;
  	
	//con emulatore 5 è un valore che tiene più stabile la posizione a discapito di micro cambiamenti
	//arrivato ha compito di smistare tra la poca differenza tra centro_richiesto_user e centro_corrente dovuta 
	//alle non idealità del paddle con la poca differenza dovuta al fatto che il paddle, dopo il movimento, deve ancora arrivare alla corretta posizione
	//centro_richiesto_user è appunto una richiesta. Si tiene in considerazione solo il centro corrente
	//da qui codice

	//paddle_cpu

			//AGGIORNAMENTI VERTICALI
	if(in_game){
		for(i=0;i<colonne_spostate_per_rit;i++){
		
			if(go_right_paddle_cpu){
				LCD_DrawLine( centro_curr_cpu-dim_base/2,y_paddle_cpu+spessore_paddle-1,  centro_curr_cpu-dim_base/2, y_paddle_cpu , Black );
				centro_curr_cpu++;
				LCD_DrawLine( centro_curr_cpu+dim_base/2,y_paddle_cpu+spessore_paddle-1,  centro_curr_cpu+dim_base/2, y_paddle_cpu , Green ); 				
			}
			else if(!go_right_paddle_cpu){
				LCD_DrawLine( centro_curr_cpu+dim_base/2,y_paddle_cpu+spessore_paddle-1,  centro_curr_cpu+dim_base/2, y_paddle_cpu , Black );
				centro_curr_cpu--;
				LCD_DrawLine( centro_curr_cpu-dim_base/2,y_paddle_cpu+spessore_paddle-1,  centro_curr_cpu-dim_base/2, y_paddle_cpu , Green ); 
			}
			
			if( centro_curr_cpu==25)
				go_right_paddle_cpu=1;
			else if(centro_curr_cpu==214)
				go_right_paddle_cpu=0;		
		}
		centro_paddle_cpu=centro_curr_cpu;
	}

	//paddle_user
	if(diff>5 || !arrivato){  		
		//AGGIORNAMENTI VERTICALI
		for(i=0;i<colonne_spostate_per_rit && centro_richiesto_user!=centro_curr_user;i++){
			if(centro_richiesto_user>centro_curr_user){
				LCD_DrawLine( centro_curr_user-dim_base/2,y_paddle_user-spessore_paddle+1,  centro_curr_user-dim_base/2, y_paddle_user , Black );
				centro_curr_user++;
				LCD_DrawLine( centro_curr_user+dim_base/2,y_paddle_user-spessore_paddle+1,  centro_curr_user+dim_base/2, y_paddle_user , Green ); //la linea va da centro - dim_linea/2 a centro + dim_linea/2
			}
			else if(centro_richiesto_user<centro_curr_user){
				LCD_DrawLine( centro_curr_user+dim_base/2,y_paddle_user-spessore_paddle+1,  centro_curr_user+dim_base/2, y_paddle_user , Black );
				centro_curr_user--;
				LCD_DrawLine( centro_curr_user-dim_base/2,y_paddle_user-spessore_paddle+1,  centro_curr_user-dim_base/2, y_paddle_user , Green ); //la linea va da centro - dim_linea/2 a centro + dim_linea/2
			}		
		}
		//PER TENERE SEMPRE COSTANTE LA VELOCITà DEL GIOCO
		if(i!=colonne_spostate_per_rit){
		for(;i<colonne_spostate_per_rit;i++){
				//tolgo e rimetto sempre allo stesso punto per mantenere le velocità
				LCD_DrawLine( centro_curr_user+dim_base/2,y_paddle_user-spessore_paddle+1,  centro_curr_user+dim_base/2, y_paddle_user , Green );
				LCD_DrawLine( centro_curr_user+dim_base/2,y_paddle_user-spessore_paddle+1,  centro_curr_user+dim_base/2, y_paddle_user , Green ); 
			}	
		}
		if(centro_richiesto_user==centro_curr_user)
			arrivato=1;
		else 
			arrivato=0;
		
		centro_paddle_user=centro_curr_user;
		
	}else{   
		if(in_game){
			//scritture random per mantenere sempre lo stesso rate della pallina
			for(i=colonne_spostate_per_rit;i>0;i--){
				//tolgo e rimetto sempre allo stesso punto per mantenere le velocità
				LCD_DrawLine( centro_curr_user+dim_base/2,y_paddle_user-spessore_paddle+1,  centro_curr_user+dim_base/2, y_paddle_user , Green );
				LCD_DrawLine( centro_curr_user+dim_base/2,y_paddle_user-spessore_paddle+1,  centro_curr_user+dim_base/2, y_paddle_user , Green ); 
			}	
		}
	}
		
	if(in_game){ 
		info_return=genera_pallina();
		//0=mi sto muovendo nello spazio, nulla di rilevante				
		//1=cade ad user
		//2=cade a cpu

		if(info_return==1){ //cade a user	
			punteggio_current_cpu++;
			//stampo a schermo il punteggio aggiornato di cpu
			write_text(4,White);
			info_return=0;
			init_pallina();			
		}else if(info_return==2){  //cade a cpu
			punteggio_current_user++;
			//stampo a schermo il punteggio aggiornato di user
			write_text(3,White);
			info_return=0;			
			init_pallina();
		}
		
		if(  punteggio_current_cpu==5 || punteggio_current_user==5			){
			//stampo scritta lose/win			
			write_text(2,White);
			info_return=1;
		}
	}
		
	return info_return; //ritorno 0 se è tutto ok e sto continuando a giocare, 1 se ho finito il gioco
}


void init_pallina(void){
	int i;
	for(i=0;i<5;i++)
		LCD_DrawLine( x-2,y-2+i ,x+2,y-2+i,Black);	
	
	x=MAX_X-1-5-2; 
	y=MAX_Y/2;
			
	//STAMPO A SCHERMO PALLINA IN POSIZIONE INIZIALE
	for(i=0;i<5;i++){
		LCD_DrawLine( x-2,y-2+i ,x+2,y-2+i,Yellow);
	}
}


//genera_pallina ritorna info sul gioco. 
//ritorna 0 se non succede nulla, 
//1 se la pallina è caduta ad user 
//2 se la pallina è caduta a cpu

int genera_pallina(void){ 
	volatile unsigned int info_return=0;
	unsigned int i;
	static int go_down=1,go_right=1;
	static unsigned salto_x=max_salto_pixel;
	static unsigned salto_y=max_salto_pixel;
	
	volatile int sx_paddle,dx_paddle;
	static unsigned int angolo;
	
	//cancello vecchia posizione pallina
	if(  ! ( (  x>=10 && x<=(10+8*quante_cifre(punteggio_current_user))  ) &&  (y>=152 && y<=168) )      &&
			 ! ( (  x>=(227-8*quante_cifre(punteggio_max)) && x<=227    ) &&  (y>=152  && y<=168 ) )
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

	//se sono a livello del paddle user
	if(y==275){ 
		sx_paddle=centro_paddle_user-dim_base/2;
		dx_paddle=centro_paddle_user+dim_base/2;
		
		if( ( (x-2)<dx_paddle && (x+2)>dx_paddle ) || ( (x+2)<=dx_paddle && (x-2)>=sx_paddle ) || ( (x-2)<sx_paddle && (x+2)>sx_paddle )   ){ //se tocco il paddle
		
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
				if(x>centro_paddle_user){
					i=x-centro_paddle_user;
					angolo=90*i/(dim_base/2);
					if(angolo>45){
						salto_x=max_salto_pixel;
						salto_y=max_salto_pixel*(-angolo+90)/45; //per mantenere sempre un valore 0<=val<=45
						if(salto_y==0)
							salto_y=1;
					}else{
						salto_y=max_salto_pixel;
						salto_x=max_salto_pixel*angolo/45;	
					}
					go_right=1;
				}
				else{ //if(x<=centro_paddle_user){
					i=centro_paddle_user-x;
					angolo=90*i/(dim_base/2);
					if(angolo>45){
						salto_x=max_salto_pixel;
						salto_y=max_salto_pixel*(-angolo+90)/45; //per mantenere sempre un valore 0<=val<=45
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
		else{ // se non tocco il paddle
			
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
			go_down=1;
			go_right=-1;
	
			return info_return;
		}
	}

	else if(y==44){ //se tocco paddle cpu		
		sx_paddle=centro_paddle_cpu-dim_base/2;
		dx_paddle=centro_paddle_cpu+dim_base/2;

		if( ( (x-2)<dx_paddle && (x+2)>dx_paddle ) || ( (x+2)<=dx_paddle && (x-2)>=sx_paddle ) || ( (x-2)<sx_paddle && (x+2)>sx_paddle )   ){ //se tocco il paddle
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
				if(x>centro_paddle_cpu){
					i=x-centro_paddle_cpu;
					angolo=90*i/(dim_base/2);
					if(angolo>45){
						salto_x=max_salto_pixel;
						salto_y=max_salto_pixel*(-angolo+90)/45; //per mantenere sempre un valore 0<=val<=45
						if(salto_y==0)
							salto_y=1;
					}else{
						salto_y=max_salto_pixel;
						salto_x=max_salto_pixel*angolo/45;	
					}
					go_right=1;
				}
				else{ //if(x<=centro_paddle_cpu){
					i=centro_paddle_cpu-x;
					angolo=90*i/(dim_base/2);
					if(angolo>45){
						salto_x=max_salto_pixel;
						salto_y=max_salto_pixel*(-angolo+90)/45; //per mantenere sempre un valore 0<=val<=45
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
			
			go_down=1; //per andare giù
		}
		else{ //se non tocco il paddle
			info_return=2;
			 //salgo un po' su e stampo la pallina lì
			for(i=0;i<5;i++)
				LCD_DrawLine( x-2,y-2-i ,x+2,y-2-i,Yellow);
			
			for(i=0;i<20;i++){
				LCD_DrawLine( x-2,y+2 ,x+2,y+2, Black);
				y--;
				LCD_DrawLine( x-2,y-2 ,x+2,y-2,Yellow);			
			}
			//ora prepara i nuovi valori di diagonale
			salto_x=max_salto_pixel;
			salto_y=max_salto_pixel;
			go_down=1;
			go_right=-1;

			return info_return;
		}
	}
	
	if(x==7){
		go_right=1;
		suona(3175); //175Hz
	}
	else if(x==232){
		go_right=-1;
		suona(3175); //175Hz
	}	
	
	if(salto_y==1){ //se salto_y è 1 sono troppo lento. Raddoppia le velocità
		salto_y*=2;
		salto_x*=2;
	}
	
	y+=salto_y*go_down; 
	x+=salto_x*go_right; 
	
	if(x<7)//(ricordo che asse x parte da 0) 7 è IL VALORE CHE HA IL CENTRO DELLA PALLINA, ASSE X, PER IL TOCCO DEL MURO SX
		x=7;
	else if(x>232)//(ricordo che asse x termina a 239) 232 è IL VALORE CHE HA IL CENTRO DELLA PALLINA, ASSE X, PER IL TOCCO DEL MURO DX
		x=232;
		
	if(y<44)//(ricordo che asse y parte da 0) 44 è IL VALORE CHE HA IL CENTRO DELLA PALLINA, ASSE Y, PER IL TOCCO DEL PADDLE CPU
		y=44;
	else if(y>275) //(ricordo che asse y termina a 319) 275 è IL VALORE CHE HA IL CENTRO DELLA PALLINA, ASSE Y, PER IL TOCCO DEL PADDLE
		y=275;

	if(  ! ( (  x>=10 && x<=(10+8*quante_cifre(punteggio_current_user))  ) &&  (y>=152 && y<=168) )      &&
			 ! ( (  x>=(227-8*quante_cifre(punteggio_max)) && x<=227    ) &&  (y>=152  && y<=168 ) )
		){ //CONTROLLO DI ESSERE IN CONFINE SCRIVIBILE
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
