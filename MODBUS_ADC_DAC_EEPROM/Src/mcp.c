#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "application.h"
#include "modbus_function.h"
#include "project_spi.h"
#include "mcp.h"

#define CS_GPIO_Port_ADC GPIOD
#define CS_GPIO_Port_DAC GPIOC
#define CS_PIN_MCP_ADC GPIO_PIN_2
#define CS_PIN_MCP_DAC GPIO_PIN_12

/* ----------------------- ADC -------------------------------- */



uint8_t temp_channel[2];

uint8_t new_Receive[2];
uint16_t t;
//uint16_t Channel_0 = 1536;
//uint16_t Channel_1 = 1600;
//uint16_t Channel_2 = 1664;
//uint16_t Channel_3 = 1728;
//uint16_t Channel_4 = 1792;
//uint16_t Channel_5 = 1856;
//uint16_t Channel_6 = 1920;
//uint16_t Channel_7 = 1984;


void CS_ADC_SELECT()
{
	HAL_GPIO_WritePin(CS_GPIO_Port_ADC, CS_PIN_MCP_ADC, GPIO_PIN_RESET);
}

void CS_ADC_DESELECT()
{
	HAL_GPIO_WritePin(CS_GPIO_Port_ADC, CS_PIN_MCP_ADC, GPIO_PIN_SET);
}

uint8_t MCP_Receive[3];
uint16_t MCP_ADC;

uint16_t MCP3208_DIN_DOUT(uint16_t Channel)
{

	temp_channel[0] = (Channel >> 8) ;
	temp_channel[1] = Channel;


	HAL_SPI_TransmitReceive(&hspi1, temp_channel, MCP_Receive, 3, 10);

	new_Receive[0] = (0b00001111 & MCP_Receive[1]);
	new_Receive[1] = MCP_Receive[2];
	MCP_ADC = ((new_Receive[0] << 8) | new_Receive[1]);

	return MCP_ADC;
}


void MCP3208()
{

	CS_ADC_DESELECT();
	CS_ADC_SELECT();
	t=MCP3208_DIN_DOUT(1536);
	CS_ADC_DESELECT();

}

/* ----------------------- DAC -------------------------------- */

uint16_t DAC_Reg;
uint8_t Tem_Reg[2];


void CS_DAC_SELECT()
{
	HAL_GPIO_WritePin(CS_GPIO_Port_DAC, CS_PIN_MCP_DAC, GPIO_PIN_RESET);
}

void CS_DAC_DESELECT()
{
	HAL_GPIO_WritePin(CS_GPIO_Port_DAC, CS_PIN_MCP_DAC, GPIO_PIN_SET);
}
uint8_t temp_dac[2];

void MCP4911_Write()
{



	//DAC_Reg = MCP3208_DIN_DOUT(0);
	HAL_Delay(2);
	CS_DAC_DESELECT();
	HAL_Delay(2);
	CS_DAC_SELECT();
	DAC_Reg = 1000;
//	Tem_Reg[0] = 0x70 | DAC_Reg >> 8;
//	Tem_Reg[1] = DAC_Reg ;
	temp_dac[0] = 0x3 | (DAC_Reg >> 8);
	temp_dac[1] = DAC_Reg ;

	HAL_SPI_Transmit(&hspi1, temp_dac, 2, 10);

	CS_DAC_DESELECT();
}

/* -------------------------- END ------------------------------*/
