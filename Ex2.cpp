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
#include "heap_lock_monitor.h"
#include "DigitalIoPin.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
bool pause = false, green_on = false;
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

/* LED1 toggle thread */
static void vLEDTask1(void *pvParameters) {

	bool LedState = false;

    while (1) {

        int count = 1;

        while (count <= 3) {
            LedState = true;
            Board_LED_Set(0, LedState);
            vTaskDelay(500);
            LedState = false;
            Board_LED_Set(0, LedState);
            vTaskDelay(500);
            count++;
        }
        while (3 <= count && count <= 6) {
            LedState = true;
            Board_LED_Set(0, LedState);
            vTaskDelay(1000);
            LedState = false;
            Board_LED_Set(0, LedState);
            vTaskDelay(1000);
            count++;
        }
        while (6 <= count && count <= 9) {
            LedState = true;
            Board_LED_Set(0, LedState);
            vTaskDelay(500);
            LedState = false;
            Board_LED_Set(0, LedState);
            vTaskDelay(500);
            count++;
        }
        vTaskDelay(2000);
    }
}

/* LED2 toggle thread */
static void vLEDTask2(void *pvParameters) {

    while (1) {
        bool LedState = false;
        Board_LED_Set(1, LedState);
        vTaskDelay(14000);
        LedState = true;
        Board_LED_Set(1, LedState);
        vTaskDelay(14000);
    }
}

/* UART (or output) thread */
static void vUARTTask(void *pvParameters) {
	int tickCnt = 0, minutes = 0;
	//tickCnt represents seconds
	while (1) {
		DEBUGOUT("Time - %02d:%02d\r\n", minutes, tickCnt);
		tickCnt++;
		if (tickCnt == 60) {
			minutes++;
			tickCnt = 0;
		}

		/* About a 1s delay here */
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
	//NOTE: configTICK_RATE_HZ = 1000
	prvSetupHardware();

	heap_monitor_setup();

	xTaskCreate(vLEDTask1, "vTaskLed1",
			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vLEDTask2, "vTaskLed2",
			configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* UART output thread, simply counts seconds */
	xTaskCreate(vUARTTask, "vTaskUart",
			configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 2UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

