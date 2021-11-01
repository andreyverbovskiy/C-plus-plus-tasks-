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
#define TICKRATE_HZ1 (1000)

static volatile std::atomic_int count;
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief SysTick timer
 * @return 
 */

void SysTick_Handler(void) {
	if(count > 0) {
		 count--;
	}
}

#ifdef __cplusplus
}

#endif
void Sleep(int sle) {
	count = sle;
	while(count > 0) {
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

void led_off(){
	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);
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

	DigitalIoPin SW1(0,17,true, true, false);
	DigitalIoPin SW2(1,11,true, true, false);
	DigitalIoPin SW3(1,9,true, true, false);
	
	Sleep(1000);
	
	SW1.write(true);
	SW2.write(true);
	SW3.write(true);
	
	int c1 = 0;
	int c2 = 0;
	int c3 = 0;

	while(1) {
		if(SW1.read()) {
			Board_LED_Set(0, true);
			c1 = 1;
		}
		if(SW2.read()) {
			Board_LED_Set(1, true);
			c2 = 1;
		}
		if(SW3.read()) {
			Board_LED_Set(2, true);
			c3 = 0;
		}

		c1++;
		c2++;
		c3++;
		
		if(c1 == 100) {
			Board_LED_Set(0, false);
			Sleep(10);
			c1 = 1;
		}
		if(c2 == 100) {
			Board_LED_Set(1, false);
			Sleep(10);
			c2 = 1;
		}
		if(c3 == 100) {
			Board_LED_Set(2, false);
			Sleep(10);
			c3 = 1;
		}
		Sleep(10);
	}
	return 0 ;
}
