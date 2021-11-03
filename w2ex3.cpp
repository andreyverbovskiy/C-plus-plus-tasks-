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
#include <stdio.h>
#include <string.h>
#include <cr_section_macros.h>
#include "ITM_write.h"
#include <atomic>
#include <array>
#include <algorithm>

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

class DigitalIoPin {
public:
	DigitalIoPin(int port, int pin, bool input = true, bool pullup = true, bool invert = false);
	DigitalIoPin(const DigitalIoPin &) = delete;
	virtual ~DigitalIoPin() {};
	bool read();
	void write(bool value);
private:
	int port1;
	int pin1;
	uint32_t modefunc;
};

DigitalIoPin::DigitalIoPin(int port, int pin, bool input, bool pullup, bool invert): port1(port), pin1(pin) {
	if(pullup == true) {
		if(invert == true) {
			modefunc = (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN);
		}
		else {
			modefunc = (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN);
		}
	}
		else {
			if(invert == true) {
				modefunc = (IOCON_MODE_INACT | IOCON_DIGMODE_EN );
			}
			else {
				modefunc = (IOCON_MODE_INACT | IOCON_DIGMODE_EN | IOCON_INV_EN);
			}
		}

	Chip_IOCON_PinMuxSet(LPC_IOCON, port1, pin1, modefunc);

	if(input == true) {
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, port1, pin1);
	}
	else {
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port1, pin1);
	}
}

bool DigitalIoPin::read() {
	return Chip_GPIO_GetPinState(LPC_GPIO, port1, pin1);
}

void DigitalIoPin::write(bool state) {
	Chip_GPIO_SetPinState(LPC_GPIO, port1, pin1, !state);
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
	// Set the LED to the state of "On"

	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ1);

#endif
#endif

	CoreDebug->DEMCR |= 1 << 24;
	DWT->CTRL |= 1;

	ITM_init();

	std::array<int,50> sample;
	
	DigitalIoPin sw1_press(0,17,true, true, false);

	Sleep(1000);
	int sw1_count = 0,
			value =0,
			value2=0;
	char arr [20];
	while(1) {

		if(sw1_press.read() && sw1_count <50){
			value = DWT->CYCCNT;
			Sleep(1);
			value2 = DWT->CYCCNT;
			sample[sw1_count]= value2 - value;

			while(sw1_press.read()){
			Sleep(10);
			}
			Sleep(40);
			sw1_count++;
		}
		if(sw1_counter==50){

			std::sort(sample.begin(), sample.end());
			int sum =0;
			for(int i =0; i<50;i++){
				sum+=sample[i];
			snprintf(arr, 20, "\nTime %d", sample[i]);
		    	ITM_write(arr);
			
}
			snprintf(arr, 20, "\nAverage %d", sum/50);
		    	ITM_write(arr);
			sw1_count=0;
		}

	}
	return 0 ;
}
