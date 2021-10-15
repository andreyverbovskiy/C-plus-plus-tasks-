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
	}
	else{

		if(invert == true){
			modefunc = (IOCON_MODE_INACT | IOCON_DIGMODE_EN );

		}
		else{
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



const int DOT = 1;
const int DASH = 3;
struct MorseCode {
	char symbol;
	unsigned char code[7];
};

const MorseCode ITU_morse[] = {
		{ 'A', { DOT, DASH } }, // A
		{ 'B', { DASH, DOT, DOT, DOT } }, // B
		{ 'C', { DASH, DOT, DASH, DOT } }, // C
		{ 'D', { DASH, DOT, DOT } }, // D
		{ 'E', { DOT } }, // E
		{ 'F', { DOT, DOT, DASH, DOT } }, // F
		{ 'G', { DASH, DASH, DOT } }, // G
		{ 'H', { DOT, DOT, DOT, DOT } }, // H
		{ 'I', { DOT, DOT } }, // I
		{ 'J', { DOT, DASH, DASH, DASH } }, // J
		{ 'K', { DASH, DOT, DASH } }, // K
		{ 'L', { DOT, DASH, DOT, DOT } }, // L
		{ 'M', { DASH, DASH } }, // M
		{ 'N', { DASH, DOT } }, // N
		{ 'O', { DASH, DASH, DASH } }, // O
		{ 'P', { DOT, DASH, DASH, DOT } }, // P
		{ 'Q', { DASH, DASH, DOT, DASH } }, // Q
		{ 'R', { DOT, DASH, DOT } }, // R
		{ 'S', { DOT, DOT, DOT } }, // S
		{ 'T', { DASH } }, // T
		{ 'U', { DOT, DOT, DASH } }, // U
		{ 'V', { DOT, DOT, DOT, DASH } }, // V
		{ 'W', { DOT, DASH, DASH } }, // W
		{ 'X', { DASH, DOT, DOT, DASH } }, // X
		{ 'Y', { DASH, DOT, DASH, DASH } }, // Y
		{ 'Z', { DASH, DASH, DOT, DOT } }, // Z
		{ '1', { DOT, DASH, DASH, DASH, DASH } }, // 1
		{ '2', { DOT, DOT, DASH, DASH, DASH } }, // 2
		{ '3', { DOT, DOT, DOT, DASH, DASH } }, // 3
		{ '4', { DOT, DOT, DOT, DOT, DASH } }, // 4
		{ '5', { DOT, DOT, DOT, DOT, DOT } }, // 5
		{ '6', { DASH, DOT, DOT, DOT, DOT } }, // 6
		{ '7', { DASH, DASH, DOT, DOT, DOT } }, // 7
		{ '8', { DASH, DASH, DASH, DOT, DOT } }, // 8
		{ '9', { DASH, DASH, DASH, DASH, DOT } }, // 9
		{ '0', { DASH, DASH, DASH, DASH, DASH } }, // 0
		{ 0, { 0 } } // terminating entry - Do not remove!
};


class MorseCodeSender{
public:
	MorseCodeSender();
	virtual ~MorseCodeSender() {};
	bool display(char *text);
private:
	DigitalIoPin ob_led;
	DigitalIoPin ob_output;

};

MorseCodeSender::MorseCodeSender() :ob_led(0,25, false, true, false), ob_output(0,8, false, false, false){}

bool MorseCodeSender::display(char *text){
	
	int gap = 50; 

	for(int count = 0; count < 80 && text[count] != '\0'; count++) {
		
		if (text[count] == ' ') {

			Sleep(gap*7);
			Board_UARTPutChar('7');
		}
		else {
			for(int count2 = 0;  ITU_morse[count2].symbol != '\0'; count2++){
				if(ITU_morse[count2].symbol == text[count])
				{
					for(int x = 0;  ITU_morse[count2].code[x] == 1 || ITU_morse[count2].code[x] == 3; x++) {
						if(ITU_morse[count2].code[x]==1) {
							ob_led.write(true);
							ob_output.write(true);
							Sleep(gap);
							Board_UARTPutChar('.');
							ob_led.write(false);
							ob_output.write(false);
						}
						else
						{
							ob_led.write(true);
							ob_output.write(true);
							Sleep(gap*3);
							Board_UARTPutChar('_');
							ob_led.write(false);
							ob_output.write(false);
						}

						if(ITU_morse[count2].code[x+1] == 1 || ITU_morse[count2].code[x+1] == 3){
							Board_UARTPutChar('1');
							Sleep(gap);
						}
					}
				}
			}
			
			if(text[count+1] !='\0' && text[count+1] !=' '){

				ob_led.write(false);
				ob_output.write(false);
				Sleep(gap * 3);
				Board_UARTPutChar('3');
			}
		}
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
	// Set the LED to the state of "On"
	Board_LED_Set(0, true);
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ1);

#endif
#endif
	
	bool tog = false;

	int i = 0;
	char stor[80];

	Board_UARTPutSTR("\r\nTask2 check\r\n");
	Board_UARTPutChar('!');
	Board_UARTPutChar('\r');
	Board_UARTPutChar('\n');

	int c;
	 

	while(1) { 

		c = Board_UARTGetChar();

		if(c != EOF) {
			if(c == '\n') {

				Board_UARTPutChar('\r'); 

			}
			else if(c == '\r' || i==80) {

				Board_UARTPutChar('\n'); 
				stor[i] = '\0'; 
				i=0;
			}
			else{
				stor[i] = ((toupper(c) <='Z' && toupper(c) >='A') || toupper(c) ==' ' || (toupper(c) <='9' && toupper(c) >='0')) ?toupper(c) : 'X' ;
				Board_UARTPutChar(stor[i]);
				i++;
			}

			if(i==0 && stor[i] !='\0'){ 

				MorseCodeSender obsender;
				obsender.display(stor);
				Board_UARTPutChar('\r');
			}
		}
		Sleep(10);
	}



	return 0;
}