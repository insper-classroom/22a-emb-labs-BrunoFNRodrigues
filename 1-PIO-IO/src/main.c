/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define LED_PIO				PIOC          
#define LED_PIO_ID			ID_PIOC
#define LED_PIO_IDX			8
#define LED_PIO_IDX_MASK	(1 << LED_PIO_IDX)

// Configuracoes do botao
#define BUT_PIO				PIOA
#define BUT_PIO_ID          ID_PIOA
#define BUT_PIO_IDX			11
#define BUT_PIO_IDX_MASK (1u << BUT_PIO_IDX) // esse já está pronto.

// Placa OLED
//LEDs
#define OLED1_PIO     PIOA
#define OLED1_PIO_ID    ID_PIOA
#define OLED1_PIO_IDX    0
#define OLED1_PIO_IDX_MASK   (1 << OLED1_PIO_IDX)

#define OLED2_PIO     PIOC
#define OLED2_PIO_ID    ID_PIOC
#define OLED2_PIO_IDX    30
#define OLED2_PIO_IDX_MASK   (1 << OLED2_PIO_IDX)

#define OLED3_PIO     PIOB
#define OLED3_PIO_ID    ID_PIOB
#define OLED3_PIO_IDX    2
#define OLED3_PIO_IDX_MASK   (1 << OLED3_PIO_IDX)

//Botões
#define BUT1_PIO    PIOD
#define BUT1_PIO_ID  ID_PIOD
#define BUT1_PIO_IDX   28
#define BUT1_PIO_IDX_MASK (1u << BUT1_PIO_IDX)

#define BUT2_PIO    PIOC
#define BUT2_PIO_ID  ID_PIOC
#define BUT2_PIO_IDX   31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX)

#define BUT3_PIO    PIOA
#define BUT3_PIO_ID  ID_PIOA
#define BUT3_PIO_IDX   19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX)


/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void){
	// Initialize the board clock
	sysclk_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_PIO_ID);
	
	//Inicializa PC8 como saída
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(OLED1_PIO_ID);
	
	//Inicializa PC8 como saída
	pio_set_output(OLED1_PIO, OLED1_PIO_IDX_MASK, 0, 0, 0);
	
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(OLED2_PIO_ID);
	
	//Inicializa PC8 como saída
	pio_set_output(OLED2_PIO, OLED2_PIO_IDX_MASK, 0, 0, 0);
	
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(OLED3_PIO_ID);
	
	//Inicializa PC8 como saída
	pio_set_output(OLED3_PIO, OLED3_PIO_IDX_MASK, 0, 0, 0);
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, PIO_DEFAULT);

	pio_pull_up(BUT_PIO, BUT_PIO_IDX_MASK, 1);
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT1_PIO_ID);
		
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT1_PIO, BUT1_PIO_IDX_MASK, PIO_DEFAULT);

	pio_pull_up(BUT1_PIO, BUT1_PIO_IDX_MASK, 1);
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT2_PIO_ID);
	
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT2_PIO, BUT2_PIO_IDX_MASK, PIO_DEFAULT);

	pio_pull_up(BUT2_PIO, BUT2_PIO_IDX_MASK, 1);
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT3_PIO_ID);
	
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_set_input(BUT3_PIO, BUT3_PIO_IDX_MASK, PIO_DEFAULT);

	pio_pull_up(BUT3_PIO, BUT3_PIO_IDX_MASK, 1);


}


/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {
	if (pio_get(PIOA, PIO_INPUT, BUT1_PIO_IDX_MASK) != 1){
		int i = 0;
		for (i = 0; i < 6; ++i){
			pio_set(PIOC, LED_PIO_IDX_MASK);      // Coloca 1 no pino LED
			delay_ms(500);                        // Delay por software de 200 ms
			pio_clear(PIOC, LED_PIO_IDX_MASK);    // Coloca 0 no pino do LED
			delay_ms(500);
		}
	}
	
	if (pio_get(PIOD, PIO_INPUT, BUT_PIO_IDX_MASK) != 1){
		int i = 0;
		for (i = 0; i < 6; ++i){
			pio_set(PIOA, OLED1_PIO_IDX_MASK);      // Coloca 1 no pino LED
			delay_ms(500);                        // Delay por software de 200 ms
			pio_clear(PIOA, OLED1_PIO_IDX_MASK);    // Coloca 0 no pino do LED
			delay_ms(500);
		}
	}
		
	if (pio_get(PIOC, PIO_INPUT, BUT2_PIO_IDX_MASK) != 1){
		int i = 0;
		for (i = 0; i < 6; ++i){
			pio_set(OLED2_PIO, OLED2_PIO_IDX_MASK);      // Coloca 1 no pino LED
			delay_ms(500);                        // Delay por software de 200 ms
			pio_clear(OLED2_PIO, OLED2_PIO_IDX_MASK);    // Coloca 0 no pino do LED
			delay_ms(500);
		}
	}
		
	if (pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK) != 1){
		int i = 0;
		for (i = 0; i < 6; ++i){
			pio_set(OLED3_PIO, OLED3_PIO_IDX_MASK);      // Coloca 1 no pino LED
			delay_ms(500);                        // Delay por software de 200 ms
			pio_clear(OLED3_PIO, OLED3_PIO_IDX_MASK);    // Coloca 0 no pino do LED
			delay_ms(500);
		}
	}

  }
  return 0;
}
