#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

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

#define CLK 300000000

/************************************************************************/
/* constants                                                            */
/************************************************************************/

volatile char but_flag;
volatile char but_down;
volatile char but_led;
volatile char display_refresh;
volatile char paro;

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

int periodo = 500;
int x = 20;

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);
void io_init(void);
void pisca_led(int n);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

void but_callback(void)
{
	but_led = 1;
}

void but1_callback(void)
{
	int n = 0;
	if (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
		while (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)){
		if(n >= CLK/100){
			but_down = 1;
		}
		n++;
		}
		
    } else if (!but_down){
       	but_flag = 1;
    }
}

void but2_callback(void)
{
	paro = 1;
	x = 20;
}

void but3_callback(void)
{
	periodo += 100;
	display_refresh = 1;
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/
//Atualiza progresso da piscada
void pisca_status(int i, int n){
	int w = 90/n;
	
	if ( x == 20){
		gfx_mono_draw_filled_rect(20, 24, 90, 5, 0);
		gfx_mono_draw_rect(20, 24, 90, 5, 1);
	}
	
	gfx_mono_draw_filled_rect(x, 24, w, 5, 1);
	x += w;
	
	if ( x == 110){
		x = 20;
	}
}

//Atualiza a freq no dislpay
void display_freq(double f){
	f = 1/(f*0.002);
	char freq_str[128];
	sprintf(freq_str, "%.2f", f);
	
	gfx_mono_draw_string("f:", 10,8, &sysfont);
	gfx_mono_draw_string(freq_str, 30,8, &sysfont);
}


// pisca led N vez no periodo T
void pisca_led(int n){

	for (int i=0;i<n;i++){
		if(paro){
			paro = 0;
			break;
		}
		pisca_status(i, n);
		pio_clear(LED_PIO, LED_PIO_IDX_MASK);
		delay_ms(periodo);
		pio_set(LED_PIO, LED_PIO_IDX_MASK);
		delay_ms(periodo);	
	}
}


// Inicializa botao SW0 do kit com interrupcao
void io_init(void)
{

	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_PIO_IDX_MASK, PIO_DEFAULT);

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT1_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP|PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO,
	BUT1_PIO_ID,
	BUT1_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but1_callback);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
	
	/**************************************************/
	// BOTÂO DA PLACA
	/**************************************************/
	
	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_PIO_IDX_MASK, PIO_PULLUP|PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PIO, BUT_PIO_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO,
	BUT_PIO_ID,
	BUT_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but_callback);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
	
	/**************************************************/
	// BOTÂO DA OLED2
	/**************************************************/
	
	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT2_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP|PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT2_PIO, BUT2_PIO_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_PIO_IDX_MASK,
	PIO_IT_RISE_EDGE,
	but2_callback);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT2_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 4
	
	/**************************************************/
	// BOTÂO DA OLED3
	/**************************************************/
	
	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT3_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP|PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT3_PIO, BUT3_PIO_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_PIO_IDX_MASK,
	PIO_IT_RISE_EDGE,
	but3_callback);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT3_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

int main (void)
{
	board_init();
	sysclk_init();
	delay_init();

  // Init OLED
	gfx_mono_ssd1306_init();

  // configura botao com interrupcao
	io_init();
  

  /* Insert application code here, after the board has been initialized. */
	int n = 30;
	
	while(1) {

		if(but_flag){
			periodo += 100;
			but_flag = 0;
			display_freq(periodo);
		}
		
		if(but_down){
			if (periodo > 100){
				periodo -= 100;
			}

			but_down = 0;
			display_freq(periodo);
		}
		
		if(but_led){
			display_freq(periodo);
			pisca_led(n);
			but_led = 0;
		}
		
		if(display_refresh){
			display_freq(periodo);
			display_refresh = 0;
		}
		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI); 
	}
}
