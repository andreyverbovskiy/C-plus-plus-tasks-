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
#include "LiquidCrystal.h"
#include <string>
#include <mutex>
#include <ctime>


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
	
	SystemCoreClockUpdate();

#if !defined(NO_BOARD_LIB)
	
	Board_Init();
	Chip_RIT_Init(LPC_RITIMER);
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ1);
	
#endif
#endif
	
	DigitalIoPin rs(0,8,false,false,false);
	DigitalIoPin en(1,6,false,false,false);
	DigitalIoPin d4(1,8,false,false,false);
	DigitalIoPin d5(0,5,false,false,false);
	DigitalIoPin d6(0,6,false,false,false);
	DigitalIoPin d7(0,7,false,false,false);

	DigitalIoPin sw1(0,17,true,true,false);
	DigitalIoPin sw2(1,11,true,true,false);
	DigitalIoPin sw3(1,9,true,true,false);

	LiquidCrystal lcd(&rs,&en,&d4,&d5,&d6,&d7);

	bool sw1_state = true;
	bool sw2_state = false;
	bool sw3_state = false;

	Sleep(100);
	
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);

	bool btn_p = true;

	char arr[16];

	char str[2][5] ={ "UP  ", "DOWN"};

	lcd.setCursor(0, 0);
	lcd.print("B1   B2   B3");

	while(1){
		if(sw1.read()){
			sw1_state = (sw1_state)? false : true;
			btn_p = true;
			while(sw1.read()) Sleep(10);
		}

		if(sw2.read()){
			sw2_state = (sw2_state)? false : true;
			btn_p = true;
			while(sw2.read()) Sleep(10);
		}

		if(sw3.read()){
			sw3_state = (sw3_state)? false : true;
			btn_p = true;
			while(sw3.read()) Sleep(10);
		}
		if(btn_p == true){
			snprintf(arr, 16, "%s %s %s", str[sw1_state],str[sw2_state], str[sw3_state]);
			lcd.setCursor(0, 1);
			lcd.print(arr);
			btn_p = false;
		}

	}
	return 0 ;
}
