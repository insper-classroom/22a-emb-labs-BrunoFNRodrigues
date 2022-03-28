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

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define ECHO_PIO			PIOD          
#define ECHO_PIO_ID			ID_PIOD
#define ECHO_PIO_IDX			30
#define ECHO_PIO_IDX_MASK	(1 << ECHO_PIO_IDX)

// Configuracoes do botao
#define TRIG_PIO				PIOA
#define TRIG_PIO_ID          ID_PIOA
#define TRIG_PIO_IDX			6
#define TRIG_PIO_IDX_MASK (1u << TRIG_PIO_IDX) // esse já está pronto.

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
/* PROTOTYPES                                                           */
/************************************************************************/

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);

/************************************************************************/
/* constants                                                            */
/************************************************************************/

volatile double sound_v = 340.29;
volatile double freq = 1/(0.000058*2);

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

volatile char trig_flag = 0;
volatile char display_flag = 0;
volatile double timer = 0.0;

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);
void but_callback(void);
void echo_callback(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

void but_callback()
{
	trig_flag = 1;
}

void echo_callback()
{
	if (!pio_get(ECHO_PIO, PIO_INPUT, ECHO_PIO_IDX_MASK))
	{
		//botão levantado
		timer = rtt_read_timer_value(RTT);
		display_flag = 1;
	}else{
		RTT_init(freq, 0, 0);
	}
}

void RTT_Handler(void) {
	uint32_t ul_status;

	//Get RTT status - ACK 
	ul_status = rtt_get_status(RTT);
	

}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

// Função de inicialização do uC
void init(void){
	// Initialize the board clock
	board_init();
	sysclk_init();
	delay_init();
	gfx_mono_ssd1306_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	
	pmc_enable_periph_clk(TRIG_PIO_ID);
	pio_set_output(TRIG_PIO, TRIG_PIO_IDX_MASK, 0, 0, 0);
	
	pmc_enable_periph_clk(ECHO_PIO_ID);
	pio_set_input(ECHO_PIO, ECHO_PIO_IDX_MASK, PIO_DEFAULT);
	
	pio_handler_set(ECHO_PIO,
	ECHO_PIO_ID,
	ECHO_PIO_IDX_MASK,
	PIO_IT_EDGE,
	echo_callback);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	//pio_enable_interrupt(ECHO_PIO, ECHO_PIO_IDX_MASK);
	pio_get_interrupt_status(ECHO_PIO);
		
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(ECHO_PIO_ID);
	NVIC_SetPriority(ECHO_PIO_ID, 4); // Prioridade 4
	
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP|PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT2_PIO, BUT2_PIO_IDX_MASK, 60);
	
	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_PIO_IDX_MASK,
	PIO_IT_EDGE,
	but_callback);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT2_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 4); // Prioridade 4

}

void display_refresh()
{
	char dist_str[128];
	double dist = ((sound_v * (timer/freq))/2.0)*100;
	sprintf(dist_str, "%.2f", dist);
		
	gfx_mono_draw_string("d:", 10,8, &sysfont);
	gfx_mono_draw_string(dist_str, 30,8, &sysfont);
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

	if (trig_flag)
	{
		pio_enable_interrupt(ECHO_PIO, ECHO_PIO_IDX_MASK);
		pio_get_interrupt_status(ECHO_PIO);
		
		pio_set(TRIG_PIO, TRIG_PIO_IDX_MASK);
		delay_us(10);
		pio_clear(TRIG_PIO, TRIG_PIO_IDX_MASK);
		trig_flag = 0;
	}
	
	if (display_flag)
	{
		pio_disable_interrupt(ECHO_PIO, ECHO_PIO_IDX_MASK);
		pio_get_interrupt_status(ECHO_PIO);
		
		display_refresh();
		display_flag = 0;
	}

  }
  return 0;
}
