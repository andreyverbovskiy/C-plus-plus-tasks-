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
		green.write(led_state); // blink_frequency
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


	Sleep(100);
	/* Setup ADC for 12-bit mode and normal power */
	Chip_ADC_Init(LPC_ADC0, 0);
	/* Setup for ADC clock rate */
	Chip_ADC_SetClockRate(LPC_ADC0, 500000);

	/* For ADC0, sequencer A will be used without threshold events.
	    It will be triggered manually, convert CH8 and CH10 in the sequence  */
	Chip_ADC_SetupSequencer(LPC_ADC0, ADC_SEQA_IDX, (ADC_SEQ_CTRL_CHANSEL(8) |
			ADC_SEQ_CTRL_CHANSEL(10) | ADC_SEQ_CTRL_MODE_EOS));

	// fix this and check if this is needed
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 0, (IOCON_MODE_INACT | IOCON_ADMODE_EN));
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 0, (IOCON_MODE_INACT | IOCON_ADMODE_EN));
	/* For ADC0, select analog input pin for channel 0 on ADC0 */
	Chip_ADC_SetADC0Input(LPC_ADC0, 0);

	/* Use higher voltage trim for both ADC */
	Chip_ADC_SetTrim(LPC_ADC0, ADC_TRIM_VRANGE_HIGHV);

	/* Assign ADC0_8 to PIO1_0 via SWM (fixed pin) and ADC0_10 to PIO0_0 */
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC0_8);
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC0_10);

	/* Need to do a calibration after initialization and trim */
	//while (!(Chip_ADC_IsCalibrationDone(LPC_ADC0))); // The NXP library function violates theirown access rules given in data sheet so we can't use it
	ADC_StartCalibration(LPC_ADC0);

	/* Set maximum clock rate for ADC */
	/* Our CPU clock rate is 72 MHz and ADC clock needs to be 50 MHz or less
	 * so the divider must be at least two. The real divider used is the value below + 1
	 */
	Chip_ADC_SetDivider(LPC_ADC0, 1);

	/* Chip_ADC_SetClockRate set the divider but due to rounding error it sets the divider too low
	 * which results in a clock rate that is out of allowed range
	 */
	//Chip_ADC_SetClockRate(LPC_ADC0, 500000); // does not work with 72 MHz clock when we wantmaximum frequency

	/* Clear all pending interrupts and status flags */
	Chip_ADC_ClearFlags(LPC_ADC0, Chip_ADC_GetFlags(LPC_ADC0));

	/* Enable sequence A trimmer_readingletion interrupts for ADC0 */
	Chip_ADC_EnableInt(LPC_ADC0, ADC_INTEN_SEQA_ENABLE);
	/* We don't enable the corresponding interrupt in NVIC so the flag is set but no interrupt is
	triggered */

	/* Enable sequencer */
	Chip_ADC_EnableSequencer(LPC_ADC0, ADC_SEQA_IDX);



	uint32_t a0;
	uint32_t d0;

	char arr[100];

	int8_t temp; //temperature variable

	ITM_init();

	while(1) {

		temp = ReadTemperatureI2CM();

		Chip_ADC_StartSequencer(LPC_ADC0, ADC_SEQA_IDX);
		
		while(!(Chip_ADC_GetFlags(LPC_ADC0) & ADC_FLAGS_SEQA_INT_MASK));
		
		Chip_ADC_ClearFlags(LPC_ADC0, Chip_ADC_GetFlags(LPC_ADC0));
		
		a0 = Chip_ADC_GetDataReg(LPC_ADC0, 8); 
		d0 = ADC_DR_RESULT(a0); 

		int tr = ((int)d0 / 25) -40; //trimmer reading

		int x = tr - temp; 
		
		__disable_irq();
		if(x < 0){ 

			blink_frequency = ((40-temp)/(-x)*100);

		} 

		else if (x > 0){  

			blink_frequency = ((125-temp)/x * 100);

		} 

		else {
			blink_frequency = 0;
		}
		
		__enable_irq();
		Sleep(100);
		if(tr > temp){
			led_on =2;
		}
		else if(tr < temp){

			led_on = 1;

		} 

		else if(tr >= (temp - 1) && tr <= (temp +1)){ 

			led_on = 3;
		}		

		snprintf(arr,100, "\nRaw: %x, Dig: %d,  Freq: %d",a0,d0,(int)blink_frequency);
		ITM_write(arr);
	}
	return 0 ;

}