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
#include <string>
#include <mutex>
#include <ctime>


#include <atomic>
#define TICKRATE_HZ1 (1000)

#define PININT_IRQ_HANDLER  PIN_INT0_IRQHandler
#define PININT_NVIC_NAME    PIN_INT0_IRQn
#define PININT_INDEX   0

static volatile std::atomic_int counter;
static volatile std::atomic_int value; //counts the number of times the button was pressed

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Handle interrupt from SysTick timer
 * @return Nothing
 */

void PIN_INT0_IRQHandler(void)
{
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX));
	value++;
}

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
	int port1, pin1;
	uint32_t modefunc;
};



DigitalIoPin::DigitalIoPin(int port, int pin, bool input, bool pullup, bool invert): port1(port), pin1(pin){

	if(pullup == true){
		if(invert == true){
			modefunc = (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN);
		}else{
			modefunc = (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN);
		}
	}else{

		if(invert == true){
			modefunc = (IOCON_MODE_INACT | IOCON_DIGMODE_EN );

		}else{
			modefunc = (IOCON_MODE_INACT | IOCON_DIGMODE_EN | IOCON_INV_EN);
		}
	}

	Chip_IOCON_PinMuxSet(LPC_IOCON, port1, pin1, modefunc);

	if(input == true){
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, port1, pin1);
	}else{
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port1, pin1);
	}


}

bool DigitalIoPin::read(){
	return Chip_GPIO_GetPinState(LPC_GPIO, port1, pin1);

}

void DigitalIoPin::write(bool state){
	Chip_GPIO_SetPinState(LPC_GPIO, port1, pin1, !state);
}




int main(void) {

	uint32_t sysTickRate;

#if defined (__USE_LPCOPEN)
	
	SystemCoreClockUpdate();

#if !defined(NO_BOARD_LIB)
	
	Board_Init();
	
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ1);
	
#endif
#endif
	
    Chip_RIT_Init(LPC_RITIMER);

	//PININT set up
	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	DigitalIoPin sw1(0,17,true,true,false);

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_PININT);
	Chip_SYSCTL_PeriphReset(RESET_PININT);

	Chip_INMUX_PinIntSel(PININT_INDEX, 0, 17);

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX));

	NVIC_ClearPendingIRQ(PININT_NVIC_NAME);
	NVIC_EnableIRQ(PININT_NVIC_NAME);
	
	Sleep(100);

	char arr[80];

	int i = value; 

	while(1){
		Sleep(100);
		if(i != value){
			i = value;
			snprintf(arr, 80, "Button was pressed: %d times\r\n", i);
			Board_UARTPutSTR(arr);
		}
	}
    return 0 ;
}
