/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "heap_lock_monitor.h"
#include "DigitalIoPin.h"
#include "timers.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
TimerHandle_t timer1;
SemaphoreHandle_t semaph;

DigitalIoPin *SW1, *SW2, *SW3;


volatile bool timer_on = false;
/*****************************************************************************
 * Private functions
 ****************************************************************************/

void uart_print(const char *str){
	xSemaphoreTake(semaph, portMAX_DELAY);
	Board_UARTPutSTR(str);
	xSemaphoreGive(semaph);
}

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
}

static void vTask1(void *pvParameters) {
	while (1) {
		if (SW1->read() || SW2->read() || SW3->read()) {

			if (xTimerReset(timer1, 10) == pdPASS ) {
				xTimerStart(timer1, 0);
				Board_LED_Set(1, true);
				timer_on = true;

			}
			else{
				uart_print("Error.\r\n");
			}

		}

	}
}


void vTimer1(TimerHandle_t timer1) {
	Board_LED_Set(1, false);
	uart_print("5 seconds passed\r\n");
	timer_on = false;
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statistics collection */

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void)
{
	prvSetupHardware();
	heap_monitor_setup();
	semaph = xSemaphoreCreateMutex();
	SW1 = new DigitalIoPin(0, 17, DigitalIoPin::pullup, true);
	SW2 = new DigitalIoPin(1, 11, DigitalIoPin::pullup, true);
	SW3 = new DigitalIoPin(1, 9, DigitalIoPin::pullup, true);

	timer1 = xTimerCreate("Oneshot",  pdMS_TO_TICKS(5000), pdFALSE, (void *) 0, vTimer1);

	xTaskCreate(vTask1, "Task1",
			256, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
