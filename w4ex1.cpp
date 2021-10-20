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


static bool ReadConfigI2CM(void);

static void Init_I2C_PinMux(void){
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, IOCON_DIGMODE_EN | I2C_MODE);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, IOCON_DIGMODE_EN | I2C_MODE);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SCL);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SDA);
}

static void setupI2CMaster(){
	Chip_I2C_Init(LPC_I2C0);
	Chip_I2C_SetClockDiv(LPC_I2C0, I2C_CLK_DIVIDER);
	Chip_I2CM_SetBusSpeed(LPC_I2C0, I2C_BITRATE);
	Chip_I2CM_Enable(LPC_I2C0);
}

static void SetupXferRecAndExecute(uint8_t devAddr,
		uint8_t *txBuffPtr,
		uint16_t txSize,
		uint8_t *rxBuffPtr,
		uint16_t rxSize)
{
	i2cmXferRec.slaveAddr = devAddr;
	i2cmXferRec.status = 0;
	i2cmXferRec.txSz = txSize;
	i2cmXferRec.rxSz = rxSize;
	i2cmXferRec.txBuff = txBuffPtr;
	i2cmXferRec.rxBuff = rxBuffPtr;

	Chip_I2CM_XferBlocking(LPC_I2C0, &i2cmXferRec);
}

static void ReadTemperatureI2CM(void){
	uint8_t temperature[1];
	uint8_t tc74TempRegisterAddress = 0;

	if (ReadConfigI2CM()) {
		if(i2cmXferRec.status == I2CM_STATUS_OK){
			SetupXferRecAndExecute(
					I2C_TEMP_ADDR,
					&tc74TempRegisterAddress,
					1,
					temperature,
					1
			);
			Board_UARTPutSTR("\r\nCurrent temperature is: \r\n");
			char c[10];
			sprintf(c, "%d", (int8_t)temperature[0]);
			Board_UARTPutSTR(c);
		}
	}
	else {
		Board_UARTPutSTR("\r\nNo data yet.\r\n");
	}
}

static bool ReadConfigI2CM(void) {
	uint8_t tc74ConfigRegisterAddress = 1; // 0x01 for config
	uint8_t config[1];

	SetupXferRecAndExecute(
			I2C_TEMP_ADDR,
			&tc74ConfigRegisterAddress,
			1,
			config,
			1
	);

	if(i2cmXferRec.status == I2CM_STATUS_OK){
		if(((config[0] >> 6) & 1U )== 1){
			return true;
		} else {
			return false;
		}
	}
	return false;
}

void Set_SysTickRate(int frequency){
	uint32_t sysTickRate;
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / frequency);
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
	
	Init_I2C_PinMux();
	setupI2CMaster();
	NVIC_DisableIRQ(I2C0_IRQn);
	Set_SysTickRate(TICKRATE_HZ);

	DigitalIoPin s(1, 9, true, true, true);
	
	Sleep(100);

	while (1) {
		if (s.read()) {
			ReadTemperatureI2CM();
			Sleep(10);
		}
	}

	return 0;
}