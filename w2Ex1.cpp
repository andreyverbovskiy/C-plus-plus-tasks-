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
#include "timers.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
#define QUEUE_SIZE (5)

SemaphoreHandle_t semaph, semaph2;
QueueHandle_t queue;
TimerHandle_t xTimer1, xTimer2;
/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Print to UART with guard. Stands for Semaphore Print*/
void sem_print(const char *str){
	xSemaphoreTake(semaph2, portMAX_DELAY);
	Board_UARTPutSTR(str);
	xSemaphoreGive(semaph2);
}

static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
}

static void vTask1(void *pvParameters) {
	char command[10];
	while (1) {
		if(xQueueReceive(queue, command, portMAX_DELAY) == pdPASS) {
			sem_print(command);
		}
	}
}

void vTimer1(TimerHandle_t xTimer) {
	xSemaphoreGive(semaph);
}

static void vTask2(void *pvParameters) {
	const char *command = "Aargh\r\n";

	while (1) {
		if (xSemaphoreTake(semaph, portMAX_DELAY) == pdTRUE) {
			if(xQueueSendToBack(queue, command, portMAX_DELAY) != pdPASS ) {
				sem_print("Error, command cannot be sent.\r\n");
			}
		}
	}
}



void vTimer2(TimerHandle_t xTimer) {
	const char *command = "Hello!\r\n";

	if(xQueueSend(queue, command, portMAX_DELAY) != pdPASS ) {
		sem_print("Unable to send a command.\r\n");
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

	queue = xQueueCreate(QUEUE_SIZE, sizeof(char[10]));

	semaph = xSemaphoreCreateBinary();
	semaph2 = xSemaphoreCreateMutex();

	xTimer1 = xTimerCreate("Oneshot",  pdMS_TO_TICKS(20000), pdFALSE, (void *) 0, vTimer1);
	xTimer2 = xTimerCreate("Reload",  pdMS_TO_TICKS(5000), pdTRUE, (void *) 1, vTimer2);

	xTimerStart(xTimer1, 0);
	xTimerStart(xTimer2, 0);

	xTaskCreate(vTask1, "Task1",
			256, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2, "Task2",
			256, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
