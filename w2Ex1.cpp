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

static void vTask1(void *pvParameters) {
    int x;
    while (1) {
        x = Board_UARTGetChar();

        if (x != EOF) {
            Board_UARTPutChar(x);
            xSemaphoreGive(semaph);
        }
    }
}

static void vTask2(void *pvParameters) {
    while (1) {
        if (xSemaphoreTake(semaph, portMAX_DELAY) == pdTRUE) {

            Board_LED_Set(0, true);
            vTaskDelay(configTICK_RATE_HZ / 10);
            Board_LED_Set(0, false);
            vTaskDelay(configTICK_RATE_HZ / 10);

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
 * @brief    main routine for FreeRTOS blinky example
 * @return    Nothing, function should not exit
 */
int main(void)
{
    //NOTE: configTICK_RATE_HZ = 1000
    prvSetupHardware();

    heap_monitor_setup();

    semaph = xSemaphoreCreateBinary();

    xTaskCreate(vTask1, "vTask1_ex",
            configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
            (TaskHandle_t *) NULL);

    xTaskCreate(vTask2, "vTask2_ex",
            configMINIMAL_STACK_SIZE + 128, NULL, (tskIDLE_PRIORITY + 1UL),
            (TaskHandle_t *) NULL);

    /* Start the scheduler */
    vTaskStartScheduler();

    /* Should never arrive here */
    return 1;
}
