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
#include "queue.h"
#include "semphr.h"
#include "heap_lock_monitor.h"

#include "DigitalIoPin.h"
#include <string>

#define QUEUE_SIZE (5)
/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
QueueHandle_t xQueue;
SemaphoreHandle_t semaph;
const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Print to UART with guard. Stands for Semaphore Print*/
void sem_print(const char *str){
	xSemaphoreTake(semaph,portMAX_DELAY);
	Board_UARTPutSTR(str);
	xSemaphoreGive(semaph);
}


/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	Board_LED_Set(0, false);
}

static void vTask1(void *pvParameters) {
	int i, counter = 0;

	while (1) {
		xSemaphoreTake(semaph,portMAX_DELAY);
		i = Board_UARTGetChar();
		xSemaphoreGive(semaph);

		if (i != EOF) {
			xSemaphoreTake(semaph,portMAX_DELAY);
			Board_UARTPutChar(i);
			xSemaphoreGive(semaph);

			if (i == '\n' || i == '\r') {
				if(xQueueSendToBack(xQueue, &counter, xTicksToWait) != pdPASS ) {
					sem_print( "Something is wrong. Could not send to the queue.\r\n" );
				}
				counter = 0;
			}
			else {
				counter++;
			}
		}
	}
}

static void vTask2(void *pvParameters) {
	DigitalIoPin sw1(0, 17, DigitalIoPin::pull_up, true);
	const int end = -1;

	while (1) {
		if (sw1.read()) {
			if( xQueueSendToBack( xQueue, &end, xTicksToWait) != pdPASS ) {
				sem_print("Something is wrong. Could not send to the queue.\r\n" );
			}
			vTaskDelay(1000);
		}
	}
}

static void vTask3(void *pvParameters) {
	int lReceivedValue, sum = 0;
	char arr[50];

	while(1) {
		if(xQueueReceive(xQueue, &lReceivedValue, xTicksToWait) == pdPASS) {
			if(lReceivedValue != -1) {
				sum += lReceivedValue;
			}
			else {

				sprintf(arr, "In total there were %d characters \r\n",sum);
				sem_print(arr);
				arr[0] = '\0';

				sum = 0;
				lReceivedValue = 0;
			}
		}
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
	//NOTE: configTICK_RATE_HZ = 1000

	xQueue = xQueueCreate(QUEUE_SIZE, sizeof(int));
	semaph = xSemaphoreCreateMutex();

	prvSetupHardware();

	heap_monitor_setup();

	xTaskCreate(vTask1, "Task1",
			configMINIMAL_STACK_SIZE  + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2, "Task2",
			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask3, "Task3",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}


