/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include <asf.h>
#include <string.h>
#include "ili9341.h"
#include "lvgl.h"
#include "touch/touch.h"

/************************************************************************/
/* fonts                                                                */
/************************************************************************/

LV_FONT_DECLARE(dseg70);
LV_FONT_DECLARE(dseg50);
LV_FONT_DECLARE(dseg30);

/************************************************************************/
/* Semaphore                                                            */
/************************************************************************/

SemaphoreHandle_t xMutexLVGL;

/************************************************************************/
/* LCD / LVGL                                                           */
/************************************************************************/

#define LV_HOR_RES_MAX          (320)
#define LV_VER_RES_MAX          (240)

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];
static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
static lv_indev_drv_t indev_drv;

lv_obj_t * temp_amb;
lv_obj_t * temp_termo;
lv_obj_t * temp_rel;

/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/

#define TASK_LCD_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY            (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* lvgl                                                                 */
/************************************************************************/

static void power_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

static void menu_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

static void config_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

static void up_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	char *c;
	int temp;
	if(code == LV_EVENT_CLICKED) {
		c = lv_label_get_text(temp_termo);
		temp = atoi(c);
		lv_label_set_text_fmt(temp_termo, "%02d", temp + 1);
	}
}

static void down_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);
	char *c;
	int temp;
	if(code == LV_EVENT_CLICKED) {
		c = lv_label_get_text(temp_termo);
		temp = atoi(c);
		lv_label_set_text_fmt(temp_termo, "%02d", temp - 1);
	}
}

void lv_termostato(void) {
	// Botões
	static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_bg_color(&style, lv_color_black());
	lv_style_set_border_color(&style, lv_palette_main(LV_PALETTE_GREEN));
	lv_style_set_border_width(&style, 0);
	
	lv_obj_t * labelBtnP;
	lv_obj_t * labelBtnM;
	lv_obj_t * labelBtnC;
	lv_obj_t * labelBtnUp;
	lv_obj_t * labelBtnDown;

	lv_obj_t * btnP = lv_btn_create(lv_scr_act());
	lv_obj_t * btnM = lv_btn_create(lv_scr_act());
	lv_obj_t * btnC = lv_btn_create(lv_scr_act());
	lv_obj_t * btnUp = lv_btn_create(lv_scr_act());
	lv_obj_t * btnDown = lv_btn_create(lv_scr_act());
	
	lv_obj_add_event_cb(btnP, power_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(btnM, menu_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(btnC, config_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(btnUp, up_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(btnDown, down_handler, LV_EVENT_ALL, NULL);
	
	lv_obj_add_style(btnP, &style, 0);
	lv_obj_add_style(btnM, &style, 0);
	lv_obj_add_style(btnC, &style, 0);
	lv_obj_add_style(btnUp, &style, 0);
	lv_obj_add_style(btnDown, &style, 0);
	
	lv_obj_align(btnP, LV_ALIGN_BOTTOM_LEFT, 10, -30);
	lv_obj_align(btnM, LV_ALIGN_BOTTOM_LEFT, 60, -30);
	lv_obj_align(btnC, LV_ALIGN_BOTTOM_LEFT, 120, -30);
	lv_obj_align(btnUp, LV_ALIGN_BOTTOM_RIGHT, -50, -30);
	lv_obj_align(btnDown, LV_ALIGN_BOTTOM_RIGHT, 0, -30);

	labelBtnP = lv_label_create(btnP);
	labelBtnM = lv_label_create(btnM);
	labelBtnC = lv_label_create(btnC);
	labelBtnUp = lv_label_create(btnUp);
	labelBtnDown = lv_label_create(btnDown);
	
	lv_label_set_text(labelBtnP, "[ " LV_SYMBOL_POWER);
	lv_label_set_text(labelBtnM, "| M |");
	lv_label_set_text(labelBtnC,  LV_SYMBOL_SETTINGS " ]");
	lv_label_set_text(labelBtnUp, "[ " LV_SYMBOL_UP " |");
	lv_label_set_text(labelBtnDown,  LV_SYMBOL_DOWN " ]");

	// Valor do temperatura ambiente
	temp_amb = lv_label_create(lv_scr_act());
	lv_obj_align(temp_amb, LV_ALIGN_LEFT_MID, 35 , -45);
	lv_obj_set_style_text_font(temp_amb, &dseg70, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(temp_amb, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(temp_amb, "%02d", 24);

	// Valor do temperatura do termostato
	temp_termo = lv_label_create(lv_scr_act());
	lv_obj_align(temp_termo, LV_ALIGN_RIGHT_MID, -10 , -30);
	lv_obj_set_style_text_font(temp_termo, &dseg50, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(temp_termo, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(temp_termo, "%02d", 18);	
	
	// Valor do temperatura do termostato
	temp_rel = lv_label_create(lv_scr_act());
	lv_obj_align(temp_rel, LV_ALIGN_TOP_RIGHT, 0 , 10);
	lv_obj_set_style_text_font(temp_rel, &dseg30, LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(temp_rel, lv_color_white(), LV_STATE_DEFAULT);
	lv_label_set_text_fmt(temp_rel, "%02d:%02d", 4, 20);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

static void task_lcd(void *pvParameters) {
	int px, py;

	lv_termostato();

	for (;;)  {
		xSemaphoreTake(xMutexLVGL, portMAX_DELAY);
		lv_tick_inc(50);
		lv_task_handler();
		xSemaphoreGive(xMutexLVGL);
		vTaskDelay(50);
	}
}

/************************************************************************/
/* configs                                                              */
/************************************************************************/

static void configure_lcd(void) {
	/**LCD pin configure on SPI*/
	pio_configure_pin(LCD_SPI_MISO_PIO, LCD_SPI_MISO_FLAGS);  //
	pio_configure_pin(LCD_SPI_MOSI_PIO, LCD_SPI_MOSI_FLAGS);
	pio_configure_pin(LCD_SPI_SPCK_PIO, LCD_SPI_SPCK_FLAGS);
	pio_configure_pin(LCD_SPI_NPCS_PIO, LCD_SPI_NPCS_FLAGS);
	pio_configure_pin(LCD_SPI_RESET_PIO, LCD_SPI_RESET_FLAGS);
	pio_configure_pin(LCD_SPI_CDS_PIO, LCD_SPI_CDS_FLAGS);
	
	ili9341_init();
	ili9341_backlight_on();
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT,
	};

	/* Configure console UART. */
	stdio_serial_init(CONSOLE_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

/************************************************************************/
/* port lvgl                                                            */
/************************************************************************/

void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
	ili9341_set_top_left_limit(area->x1, area->y1);   ili9341_set_bottom_right_limit(area->x2, area->y2);
	ili9341_copy_pixels_to_screen(color_p,  (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1));
	
	/* IMPORTANT!!!
	* Inform the graphics library that you are ready with the flushing*/
	lv_disp_flush_ready(disp_drv);
}

void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data) {
	int px, py, pressed;
	
	if (readPoint(&px, &py))
		data->state = LV_INDEV_STATE_PRESSED;
	else
		data->state = LV_INDEV_STATE_RELEASED; 
	
	data->point.x = px;
	data->point.y = py;
}

void configure_lvgl(void) {
	lv_init();
	lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);
	
	lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv.flush_cb = my_flush_cb;        /*Set a flush callback to draw to the display*/
	disp_drv.hor_res = LV_HOR_RES_MAX;      /*Set the horizontal resolution in pixels*/
	disp_drv.ver_res = LV_VER_RES_MAX;      /*Set the vertical resolution in pixels*/

	lv_disp_t * disp;
	disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
	
	/* Init input on LVGL */
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = my_input_read;
	lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void) {
	/* board and sys init */
	board_init();
	sysclk_init();
	configure_console();

	/* LCd, touch and lvgl init*/
	configure_lcd();
	configure_touch();
	configure_lvgl();

	xMutexLVGL = xSemaphoreCreateMutex();
	/* Create task to control oled */
	if (xTaskCreate(task_lcd, "LCD", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create lcd task\r\n");
	}
	
	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){ }
}
