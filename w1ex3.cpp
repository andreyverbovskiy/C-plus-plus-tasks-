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
#include <array>
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

class Dice {
public:
	Dice();
	virtual ~Dice();
	void Show(int number);
	void increment();
	int get_number();
private:
	int counter;
	DigitalIoPin P1;
	DigitalIoPin P2;
	DigitalIoPin P3;
	DigitalIoPin P4;
	DigitalIoPin P5;
	DigitalIoPin P6;
	DigitalIoPin P7;

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

Dice::Dice() : counter(1), 
		P1(1, 9, false, true, false),
		P2(0, 29, false, true, false),
		P3(0, 9, false, true, false),
		P4(0, 10, false, true, false),
		P5(0, 16, false, true, false),
		P6(1, 3, false, true, false),
		P7(0, 0, false, true, false) {

}

Dice::~Dice(){}

int Dice::get_number(){
	return counter;
}
void Dice::Show(int number){
	if(number==0){
		P7.write(true);
		P1.write(true);
		P2.write(true);
		P6.write(true);
		P5.write(true);
		P4.write(true);
		P3.write(true);
	}
	else if(number==1){
		P7.write(false);
		P1.write(false);
		P2.write(false);
		P6.write(false);
		P5.write(false);
		P4.write(true);
		P3.write(false);
	}else if(number==2){
		P7.write(true);
		P1.write(false);
		P2.write(false);
		P6.write(false);
		P5.write(false);
		P4.write(false);
		P3.write(true);
	}
	else if(number==3){
		P7.write(true);
		P1.write(false);
		P2.write(false);
		P6.write(false);
		P5.write(false);
		P4.write(true);
		P3.write(true);
	}
	else if(number==4){
		P7.write(true);
		P1.write(true);
		P2.write(false);
		P6.write(false);
		P5.write(true);
		P4.write(false);
		P3.write(true);
	}
	else if(number==5){
		P7.write(true);
		P1.write(true);
		P2.write(false);
		P6.write(false);
		P5.write(true);
		P4.write(true);
		P3.write(true);
	}
	else if(number==6){
		P7.write(true);
		P1.write(true);
		P2.write(true);
		P6.write(true);
		P5.write(true);
		P4.write(false);
		P3.write(true);
	}
	else{ //false for each one
		P1.write(false);
		P2.write(false);
		P3.write(false);
		P4.write(false);
		P5.write(false);
		P6.write(false);
		P7.write(false);
	}
}

void Dice::increment(){

	if(counter <6){
		counter++;
	}else{
		counter = 1;
	}
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


	DigitalIoPin sw0(0,8,true, true, false);
	DigitalIoPin sw1(1,6,true, true, false);
	
	Sleep(1000);
	
	sw.write(true);
	sw1.write(true);

	Dice di;


	int press_sw0 = 0;
	int press_sw1 = 0;

	while(1) {
		int i;

		if(sw0.read()){
			di.Show(0);
			press_sw0 =1;
		}
		if(!sw0.read() && press_sw0 ==1){
			di.Show(7);
			press_sw0=0;
		}
		while(sw1.read()){
			di.increment();
			press_sw1 =1;
		}
		if(!sw0.read() && press_sw1 ==1){
			i =ob.get_number();
			di.Show(i);
			press_sw1=0;
		}
		Sleep(10);

	}
	return 0 ;
}