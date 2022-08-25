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


#define CS_GPIO_Port GPIOB
#define CS_Pin GPIO_PIN_6

// Adres değeri 3 byte ve 2 byte lik olan eepromlar


uint16_t ADDR = 3;
uint8_t VALUE = 70;

uint8_t WRITE_CMD = 2;

uint8_t WRDI = 4;
uint8_t WREN = 6;
uint8_t RDSR = 5;
uint8_t WRSR = 1;


uint8_t READ_CMD = 3;
uint8_t Send_Data[32];
uint8_t get_data[32];
uint8_t SPI_Data[32];

/* ------------------------ INITIAL SETTINGS ------------------------------*/

/* SPI Chip Select */
void SELECT()
{
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}

/* SPI Chip Deselect */
void DESELECT()
{
  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

/* Command for Read or Write */
void SPI_Command(uint8_t cmd)
{
	HAL_SPI_Transmit(&hspi1,&cmd,1,10);
}

/* --------------------------- END -------------------------------*/

/* ------------------------ u8 -------------------------- */

void Write_SEQ_u8(uint16_t addr, uint8_t data)
{

	SPI_Data[0] = addr >> 8;
	SPI_Data[1] = addr;
	SPI_Command(WRITE_CMD);
	HAL_SPI_Transmit(&hspi1, SPI_Data, 2, 10);
	HAL_SPI_Transmit(&hspi1, &data, 1, 10);
}

uint8_t data;

uint8_t Read_SEQ_u8(uint16_t addr)
{

	SPI_Data[0] = READ_CMD;
	SPI_Data[1] = addr >> 8;
	SPI_Data[2] = addr;

	HAL_SPI_Transmit(&hspi1, SPI_Data,3,10);
	HAL_SPI_Receive(&hspi1,&data, 1,10);

	return data;
}

/* --------------------------- END -------------------------------*/

/* ------------------------ u16 -------------------------- */


void Write_SEQ_u16(uint16_t addr, uint16_t data)
{


	SPI_Data[0] = addr >> 8;
	SPI_Data[1] = addr;
	Send_Data[0] = data >> 8;
	Send_Data[1] = data;

	SPI_Command(WRITE_CMD);
	HAL_SPI_Transmit(&hspi1, SPI_Data, 2, 10);
	HAL_SPI_Transmit(&hspi1, Send_Data, 1, 10);
}



uint16_t Read_SEQ_u16(uint16_t addr)
{

	uint16_t Receive_data;
	SPI_Data[0] = READ_CMD;
	SPI_Data[1] = addr >> 8;
	SPI_Data[2] = addr;

	HAL_SPI_Transmit(&hspi1, SPI_Data,3,10);
	HAL_SPI_Receive(&hspi1,get_data, 2,10);


	Receive_data = (get_data[1] | (get_data[0] << 8));

	return Receive_data;

}

/* --------------------------- END -------------------------------*/

/* ------------------------ u32 -------------------------- */

void Write_SEQ_u32(uint16_t addr, uint32_t data)
{

	SPI_Data[0] = addr >> 8;
	SPI_Data[1] = addr;

	Send_Data[0] = data >>24;
	Send_Data[1] = data >> 16;
	Send_Data[2] = data >> 8;
	Send_Data[3] = data ;

	SPI_Command(WRITE_CMD);
	HAL_SPI_Transmit(&hspi1, SPI_Data, 2, 10);
	HAL_SPI_Transmit(&hspi1,Send_Data, 4, 10);
}

uint8_t data;

uint32_t Read_SEQ_u32(uint16_t addr)
{
	uint32_t Receive_data;
	SPI_Data[0] = READ_CMD;
	SPI_Data[1] = addr >> 8;
	SPI_Data[2] = addr;

	HAL_SPI_Transmit(&hspi1, SPI_Data,3,10);
	HAL_SPI_Receive(&hspi1,get_data, 4,10);

	Receive_data = (get_data[3] | (get_data[2] << 8) | (get_data[1] << 16) | (get_data[0] << 24));
	return Receive_data;
}

/* --------------------------- END -------------------------------*/
/* ----------------------- PROCESS ----------------------------*/
uint8_t okunan[2];
int i = 0;
void Process_Spi()
{
	DESELECT();
//	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 1)
//	{
	if (i== 0)
	{
		SELECT();
		SPI_Command(WREN);
		DESELECT();
		SELECT();
		Write_SEQ_u8(ADDR, VALUE);
		// SPI_Command(WRDI);
		DESELECT();
		HAL_Delay(5);
		SELECT();
		okunan[0] = 88;
		okunan[1] = Read_SEQ_u8(ADDR);
		DESELECT();

	}
	i++;

		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, RESET);
//	}

}

/* ------------------------ END ------------------------------*/
