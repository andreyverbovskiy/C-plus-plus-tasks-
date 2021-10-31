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
#include "ITM_write.h"

#include <atomic>
#define TICKRATE_HZ1 (1000)

static I2CM_XFER_T  i2cmXferRec;
#define I2C_CLK_DIVIDER (1440)
#define I2C_BITRATE (50000)
#define I2C_MODE (0)

#if defined(BOARD_NXP_LPCXPRESSO_1549)
/** 7-bit I2C addresses of Temperature Sensor */
#define I2C_TEMP_ADDR_7BIT  (0x4A)
#endif

#define I2C_STMPE811_ADDR_7BIT 0x4A

static volatile bool led_state;
static DigitalIoPin red(0,25,false, true, false);
static DigitalIoPin green(0,3,false, true, false);
static DigitalIoPin blue(1,1,false, true, false);
static volatile int led_on;
static volatile std::atomic_int counter;
static volatile std::atomic_int blink_counter;
static volatile std::atomic_int blink_frequency {0};

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
	blink_counter++;
	if(blink_counter >= blink_frequency){
		blink_counter=0;
		led_state = !led_state;
	}
	if(led_on==1){
		blue.write(led_state); 
		green.write(false);
		red.write(false);
	}
	else if(led_on==2){
		red.write(led_state); 
		green.write(false);
		blue.write(false);
	} else if(led_on==3){ 
		green.write(led_state); 
		blue.write(false);
		red.write(false);
	}
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


static void Init_I2C_PinMux(void){
#if defined(BOARD_KEIL_MCB1500)||defined(BOARD_NXP_LPCXPRESSO_1549)
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, IOCON_DIGMODE_EN | I2C_MODE);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, IOCON_DIGMODE_EN | I2C_MODE);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SCL);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SDA);
#else
#error "No I2C Pin Muxing defined for this example"
#endif
}

/* Setup I2C handle and parameters */
static void setupI2CMaster()
{
	/* Enable I2C clock and reset I2C peripheral - the boot ROM does not
	   do this */
	Chip_I2C_Init(LPC_I2C0);

	/* Setup clock rate for I2C */
	Chip_I2C_SetClockDiv(LPC_I2C0, I2C_CLK_DIVIDER);

	/* Setup I2CM transfer rate */
	Chip_I2CM_SetBusSpeed(LPC_I2C0, I2C_BITRATE);

	/* Enable Master Mode */
	Chip_I2CM_Enable(LPC_I2C0);
}

static void SetupXferRecAndExecute(uint8_t devAddr,
		uint8_t *txBuffPtr,
		uint16_t txSize,
		uint8_t *rxBuffPtr,
		uint16_t rxSize)
{
	/* Setup I2C transfer record */
	i2cmXferRec.slaveAddr = devAddr;
	i2cmXferRec.status = 0;
	i2cmXferRec.txSz = txSize;
	i2cmXferRec.rxSz = rxSize;
	i2cmXferRec.txBuff = txBuffPtr;
	i2cmXferRec.rxBuff = rxBuffPtr;

	Chip_I2CM_XferBlocking(LPC_I2C0, &i2cmXferRec);
}

/* Master I2CM receive in polling mode */
#if defined(BOARD_NXP_LPCXPRESSO_1549)
/* Function to read LM75 I2C temperature sensor and output result */

static int ReadState(void)
{
	uint8_t idAndRevision[1]; // 0x01 for config
	uint8_t registerAddress = 0;

	/* Read STMPE811 CHIP_ID and ID_VER registers */
	SetupXferRecAndExecute(
			I2C_TEMP_ADDR_7BIT,
			idAndRevision, 2,
			NULL,0);

	/* Test for valid operation */
	if (i2cmXferRec.status == I2CM_STATUS_OK) {
		return 1;
	}
	else {
		return 0;
	}
}

void set_standby(uint8_t mode){
	uint8_t tc74ConfigRegisterAddress = 1;
	// to write to the register, two bytes back to back have to be sent
	uint8_t arr[2] = {tc74ConfigRegisterAddress,mode};
	SetupXferRecAndExecute(
			I2C_TEMP_ADDR_7BIT,
			arr, 2,
			NULL,0);
}

static int ReadTemperatureI2CM(void)
{
	uint8_t temperature[1];
	uint8_t Tc74TempRegisterAddress = 0;
	uint8_t txSz = 1;

	/* Read LM75 temperature sensor */
	SetupXferRecAndExecute(
			/* The LM75 I2C bus address */
			I2C_TEMP_ADDR_7BIT,
			/* Transmit one byte, the LM75 temp register address */
			&Tc74TempRegisterAddress, 1,
			/* Receive back two bytes, the contents of the temperature register */
			temperature, 2);
	/* Test for valid operation */
	if (i2cmXferRec.status == I2CM_STATUS_OK) {

		/* Output temperature. */
		if(ReadState()==1){
			return (int8_t) temperature[0];
		}
		else{
			return -400; // not able to read value

		}


	}
	else {
		return -404; // error
	}
}
#endif
/* Start ADC calibration */
void ADC_StartCalibration(LPC_ADC_T *pADC)
{
	// clock divider is the lowest 8 bits of the control register
	/* Setup ADC for about 500KHz (per UM) */
	uint32_t ctl = (Chip_Clock_GetSystemClockRate() / 500000) - 1;

	/* Set calibration mode */
	ctl |= ADC_CR_CALMODEBIT;

	pADC->CTRL = ctl;

	/* Calibration is only trimmer_readinglete when ADC_CR_CALMODEBIT bit has cleared */
	while(pADC->CTRL & ADC_CR_CALMODEBIT) { };
}

int main(void) {

	uint32_t sysTickRate;

#if defined (__USE_LPCOPEN)
	
	SystemCoreClockUpdate();

#if !defined(NO_BOARD_LIB)
	
	Board_Init();
	
	Init_I2C_PinMux();
	
	setupI2CMaster();
	
	NVIC_DisableIRQ(I2C0_IRQn);
	
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ1);
	
#endif
#endif
	
    Chip_RIT_Init(LPC_RITIMER);

	CoreDebug->DEMCR |= 1 << 24;
	DWT->CTRL |= 1;
	DigitalIoPin rs(0,8,false,false,false);
	DigitalIoPin en(1,6,false,false,false);
	DigitalIoPin d4(1,8,false,false,false);
	DigitalIoPin d5(0,5,false,false,false);
	DigitalIoPin d6(0,6,false,false,false);
	DigitalIoPin d7(0,7,false,false,false);

	LiquidCrystal lcd(&rs,&en,&d4,&d5,&d6,&d7);
	lcd.begin(16, 2);
	lcd.setCursor(0, 0);
	BarGraph bargraph(&lcd,16,true);

	Sleep(100);


	char arr[16];
	int8_t arrtemp[16]; //to store temp values

	int8_t temp = 0; //temperature variable

	ITM_init();

	while(1) {

		temp = ReadTemperatureI2CM();
		for(int i = 0; i < 15 ;i++){
			arrtemp[i]=arrtemp[i+1];
		}
		lcd.setCursor(0,0);
		arrtemp[15]=temp;
		sprintf(arr,"%d          %d",arrtemp[14],arrtemp[15]);
		lcd.print(arr);
		lcd.setCursor(0,1);
		int value;

		for(int i = 0; i < 16; i++){
			if(arrtemp[i]<=28 && arrtemp[i]>=20){
				value = arrtemp[i]-20;
			} else if (arrtemp[i] > 28) {
				value = 8;
			} else value = 0;
			bargraph.draw(value);
		}
		Sleep(5000);
	}
	return 0 ;

}