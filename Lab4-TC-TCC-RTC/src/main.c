#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/*	Defines	*/

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;

#define LED_PIO				PIOC
#define LED_PIO_ID			ID_PIOC
#define LED_PIO_IDX			8
#define LED_PIO_IDX_MASK	(1 << LED_PIO_IDX)

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

//Bot�es
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
/* GLOBAL VAR                                                           */
/************************************************************************/

volatile char flag_rtc_alarm = 0;
volatile char flag_set_alarm = 0;
volatile char flag_relogio = 0;
volatile char flag_but3 = 0;
volatile char flag_rtc_alarm_2 = 0;
int segundo = 0;
int minuto = 0;
int hora = 0;

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void LED_init(int estado);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);
void pin_toggle(Pio *pio, uint32_t mask);
void pisca_led(int n, int t);

/************************************************************************/
/* Handlers                                                             */
/************************************************************************/

void TC0_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrup��o foi satisfeita.
	* Isso � realizado pela leitura do status do perif�rico
	**/
	volatile uint32_t status = tc_get_status(TC0, 0);
	
	/** Muda o estado do LED (pisca) **/
	
	flag_relogio = 1;
	 
}

void TC1_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrup��o foi satisfeita.
	* Isso � realizado pela leitura do status do perif�rico
	**/
	volatile uint32_t status = tc_get_status(TC0, 1);
	
	/** Muda o estado do LED (pisca) **/
	
	pin_toggle(OLED1_PIO, OLED1_PIO_IDX_MASK);
	 
}

void TC2_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrup��o foi satisfeita.
	* Isso � realizado pela leitura do status do perif�rico
	**/
	
	volatile uint32_t status = tc_get_status(TC0, 2);
	
	/** Muda o estado do LED (pisca) **/
	pin_toggle(LED_PIO, LED_PIO_IDX_MASK);
	
}

void TC3_Handler(void) {
	/**
	* Devemos indicar ao TC que a interrup��o foi satisfeita.
	* Isso � realizado pela leitura do status do perif�rico
	**/
	
	volatile uint32_t status = tc_get_status(TC1, 0);
	
	/** Muda o estado do LED (pisca) **/
	pin_toggle(OLED3_PIO, OLED3_PIO_IDX_MASK);
	
}

void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);
	
	/* IRQ due to Time has changed */
	
	pin_toggle(OLED2_PIO, OLED2_PIO_IDX_MASK);    // BLINK Led
	

}

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* seccond tick */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		// o c�digo para irq de segundo vem aqui
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		// o c�digo para irq de alame vem aqui
		if(flag_but3){
			flag_rtc_alarm = 1;	
		}else{
			flag_rtc_alarm_2 = 1;
			flag_but3 = 0;
		}
		
	}

	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}

void but3_callback()
{
	flag_set_alarm = 1;
	flag_but3 = 1;
}

void but1_callback()
{
	flag_set_alarm = 1;
	
}	

/************************************************************************/
/* Functions                                                            */
/************************************************************************/

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrup�c�o no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}

static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
}	

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

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}


void LED_init(int estado) {
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, estado, 0, 0);
	
	pmc_enable_periph_clk(OLED1_PIO_ID);
	pio_set_output(OLED1_PIO, OLED1_PIO_IDX_MASK, estado, 0, 0);
	
	pmc_enable_periph_clk(OLED2_PIO_ID);
	pio_set_output(OLED2_PIO, OLED2_PIO_IDX_MASK, estado, 0, 0);
	
	pmc_enable_periph_clk(OLED3_PIO_ID);
	pio_set_output(OLED3_PIO, OLED3_PIO_IDX_MASK, estado, 0, 0);
}

void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

void pisca_led (int n, int t) {
	for (int i=0;i<n;i++){
		pio_clear(OLED3_PIO, OLED3_PIO_IDX_MASK);
		delay_ms(t);
		pio_set(OLED3_PIO, OLED3_PIO_IDX_MASK);
		delay_ms(t);
	}
}

void BUT_init()
{
	// Inicializa clock do perif�rico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT3_PIO_ID);

	// Configura PIO para lidar com o pino do bot�o como entrada
	// com pull-up
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP|PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT3_PIO, BUT3_PIO_IDX_MASK, 60);

	// Configura interrup��o no pino referente ao botao e associa
	// fun��o de callback caso uma interrup��o for gerada
	// a fun��o de callback � a: but_callback()
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but3_callback);

	// Ativa interrup��o e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT3_PIO);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais pr�ximo de 0 maior)
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4); // Prioridade 4	
	
	// Inicializa clock do perif�rico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT1_PIO_ID);

	// Configura PIO para lidar com o pino do bot�o como entrada
	// com pull-up
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP|PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);

	// Configura interrup��o no pino referente ao botao e associa
	// fun��o de callback caso uma interrup��o for gerada
	// a fun��o de callback � a: but_callback()
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but1_callback);

	// Ativa interrup��o e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);

	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais pr�ximo de 0 maior)
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4); // Prioridade 4
}

void display_hora(){
	segundo++;
	if(segundo == 60)
	{
		segundo = 0;
		minuto++;
		gfx_mono_draw_string(" ", 90,8, &sysfont);
		if(minuto == 60)
		{
			minuto = 0;
			hora++;
			gfx_mono_draw_string(" ", 60,8, &sysfont);
			if(hora == 24)
			{
				hora = 0;
				gfx_mono_draw_string(" ", 30,8, &sysfont);
			}
		}
	}
	
	char hora_str[128];
	sprintf(hora_str, "%d", hora);
	
	char minuto_str[128];
	sprintf(minuto_str, "%d", minuto);
	
	char segundo_str[128];
	sprintf(segundo_str, "%d", segundo);

	
	gfx_mono_draw_string(hora_str, 20,8, &sysfont);
	gfx_mono_draw_string(":", 40,8, &sysfont);
	gfx_mono_draw_string(minuto_str, 50,8, &sysfont);
	gfx_mono_draw_string(":", 70,8, &sysfont);
	gfx_mono_draw_string(segundo_str, 80,8, &sysfont);
}

int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	LED_init(1);
	BUT_init();

  // Init OLED
	gfx_mono_ssd1306_init();

	TC_init(TC0, ID_TC0, 0, 1);
	tc_start(TC0, 0);

	TC_init(TC0, ID_TC1, 1, 4);
	tc_start(TC0, 1);
	
	TC_init(TC0, ID_TC2, 2, 5);
	tc_start(TC0, 2);
	
	RTT_init(1, 4, RTT_MR_RTTINCIEN);
	
    /** Configura RTC */
    calendar rtc_initial = {2018, 3, 19, 12, 15, 45 ,1};
    RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN);
   
   display_hora();

  /* Insert application code here, after the board has been initialized. */
  int count = 0;
	while(1) {
		
		if(flag_relogio)
		{
			display_hora();
			flag_relogio = 0;
		}

		if (flag_set_alarm)
		{
			/* configura alarme do RTC para daqui 20 segundos */
			uint32_t current_hour, current_min, current_sec;
			uint32_t current_year, current_month, current_day, current_week;
			rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
			rtc_get_date(RTC, &current_year, &current_month, &current_day, &current_week);
			rtc_set_date_alarm(RTC, 1, current_month, 1, current_day);
			if (current_sec > 40){
				rtc_set_time_alarm(RTC, 1, current_hour, 1, current_min + 1, 1, current_sec - 40);
			}else{
				rtc_set_time_alarm(RTC, 1, current_hour, 1, current_min, 1, current_sec + 20);
			}
			flag_set_alarm = 0;
		}
	
		
		if(flag_rtc_alarm){
			pisca_led(5, 200);
			flag_rtc_alarm = 0;
		}
		
		if(flag_rtc_alarm_2)
		{
			TC_init(TC1, ID_TC3, 0, 10);
			tc_start(TC1, 0);
			count = segundo;
			flag_rtc_alarm_2 = 0;
		}else if(segundo - count >= 10 || count-60+segundo >= 30){
			tc_stop(TC1, 0);
		}
		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);	
	}
}
