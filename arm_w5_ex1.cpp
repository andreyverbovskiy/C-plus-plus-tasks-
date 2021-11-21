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
#include "time.h"


#include "DigitalIoPin.h"
#include <string>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

DigitalIoPin *sw1lim, *sw2lim, *step, *b1, *b3, *dir;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

static void prvSetupHardware(void)
{
    SystemCoreClockUpdate();
    Board_Init();
}

static void vTask1(void *pvParameters) {

    sw1lim = new DigitalIoPin(0, 27, DigitalIoPin::pullup, true);

    sw2lim = new DigitalIoPin(0, 28,  DigitalIoPin::pullup, true);


    while (1) {
        while (sw1lim->read()) {

            Board_LED_Set(0, true);

        }

        Board_LED_Set(0, false);

        while (sw2lim->read()) {

            Board_LED_Set(1, true);

        }

        Board_LED_Set(1, false);
    }
}

static void vTask2(void *pvParameters) {

    step = new DigitalIoPin(0, 24, DigitalIoPin::output, false);
    dir = new DigitalIoPin(1, 0, DigitalIoPin::output, false);
    b1 = new DigitalIoPin(0, 17, DigitalIoPin::pullup, true);
    b3 = new DigitalIoPin(1, 9, DigitalIoPin::pullup, true);

    while (1) {
        while (b1->read() && !b3->read() && !sw1lim->read()) {

        	dir->write(true);

            step->write(true);
            vTaskDelay(5);
            step->write(false);

        }
        while (b3->read() && !b1->read() && !sw2lim->read()) {

            dir->write(false);
            step->write(true);
            vTaskDelay(5);
            step->write(false);

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
    LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L;
}

}
int main(void)
{
    prvSetupHardware();
    heap_monitor_setup();

    xTaskCreate(vTask1, "task1", 256, NULL, 1, (TaskHandle_t *) NULL);
    xTaskCreate(vTask2, "task2", 256, NULL, 1, (TaskHandle_t *) NULL);

    /* Start the scheduler */
    vTaskStartScheduler();

    /* Should never arrive here */
    return 1;
}
