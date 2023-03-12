#include <string.h>


#define dim_base 40																												 
#define y_paddle  287 	//Y: 0->319 => 319-32=287
#define spessore_paddle 10
#define max_salto_pixel 6
#define colonne_spostate_per_rit 4


/* lib_gestione_pong.c */

int calcolo_centro(unsigned short val_potenziometro);
int gioco_pong(int in_game);
int genera_pallina(void);
void pong_init(void);
void write_text(int situazione,uint16_t Color);
unsigned int quante_cifre(int valore);
