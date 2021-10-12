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
#include <atomic>
#define TICKRATE_HZ1 (1000)	/* 15 ticks per second */

static volatile std::atomic_int counter;
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Handle interrupt from SysTick timer
 * @return Nothing
 */
void SysTick_Handler(void)
{
	if(counter > 0) counter--;
}

#ifdef __cplusplus
}

#endif
void Sleep(int ms)
{
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}

int main(void) {
	uint32_t sysTickRate;
#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();

#endif
#endif
	Chip_Clock_SetSysTickClockDiv(1);

	sysTickRate = Chip_Clock_GetSysTickClockRate();

	SysTick_Config(sysTickRate / TICKRATE_HZ1);

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 17, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));

	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 17);


	while(1) {

		if(Chip_GPIO_GetPinState(LPC_GPIO, 0, 17)){
			Board_LED_Set(0, true);
			Sleep(1000);
			Board_LED_Set(0, false);
			Board_LED_Set(1, true);
			Sleep(1000);
			Board_LED_Set(1, false);
			Board_LED_Set(2, true);
			Sleep(1000);
			Board_LED_Set(2, false);
		}

	}
	return 0 ;
}



