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
#include <cr_section_macros.h>
#include "ITM_write.h"
#include <atomic>
#define TICKRATE_HZ1 (1000)
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
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
	Board_LED_Set(0, true);
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ1);

#endif
#endif
	
	bool tog = false;

	DigitalIoPin t_b(0,17,true, true, false);
	DigitalIoPin l_u(0,25,false, true, false);
	DigitalIoPin l_low(0,3,false, true, false);
	
	l_u.write(tog); 
	l_low.write(!tog); 

	Board_UARTPutSTR("\r\nTask1 check\r\n");
	Board_UARTPutChar('!');
	Board_UARTPutChar('\r');
	Board_UARTPutChar('\n');

	int c;
	while(1) { 
		if(t_b.read()){
			if(tog){
				tog = false; 
			}
			else{
				tog = true; 
			}
			while(t_b.read()){
			   Sleep(10);
			} 

		l_u.write(tog); 
		l_low.write(!tog);

		}

		c = Board_UARTGetChar();

		if(c != EOF) {
			if(c == '\n' || c == '\r'){

				if(c == '\n') Board_UARTPutChar('\r'); 
				if(c == '\r') Board_UARTPutChar('\n'); 
			}
			else{

				if(tog){

					if( (c >= 'a' || c <= 'z')){

						Board_UARTPutChar(toupper(c));

					}else{
						Board_UARTPutChar(c);
					}
				}
				else{

					if( (c >= 'A' || c <= 'Z')){
						Board_UARTPutChar(tolower(c));
					}else{
						Board_UARTPutChar(c);
					}
				}
			}

		}
		Sleep(100);
	}



	return 0;
}
