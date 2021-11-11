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

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "heap_lock_monitor.h"
#include "DigitalIoPin.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/


SemaphoreHandle_t semaph;


/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

static void vSW1Task(void *pvParameters) {

	DigitalIoPin sw1(0, 17, DigitalIoPin::pullup);

	while (1) {
		if(!sw1.read()) {

			xSemaphoreTake(semaph, portMAX_DELAY);
			DEBUGOUT("SW1 Pressed\r\n");
			xSemaphoreGive(semaph);

		}
	vTaskDelay(configTICK_RATE_HZ);
	}
}

static void vSW2Task(void *pvParameters) {

	DigitalIoPin sw2(1, 11, DigitalIoPin::pullup);

	while (1) {
		if(!sw2.read()) {

			xSemaphoreTake(semaph, portMAX_DELAY);
			DEBUGOUT("SW2 Pressed\r\n");
			xSemaphoreGive(semaph);
		}
		vTaskDelay(configTICK_RATE_HZ);
	}
}

static void vSW3Task(void *pvParameters) {

	DigitalIoPin sw3(1, 9, DigitalIoPin::pullup);

	while (1) {
		if(!sw3.read()) {

			xSemaphoreTake(semaph, portMAX_DELAY);
			DEBUGOUT("SW3 Pressed\r\n");
			xSemaphoreGive(semaph);

		}
		vTaskDelay(configTICK_RATE_HZ);
	}
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

	xTaskCreate(vSW1Task, "vTaskSW1",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vSW2Task, "vTaskSW2",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vSW3Task, "vTaskSW3",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

