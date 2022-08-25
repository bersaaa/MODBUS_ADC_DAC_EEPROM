#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include "application.h"
#include "modbus_function.h"

#define ON 1
#define OFF 0

#define MAX_SIZE 128
#define MIN_SIZE 8

uint16_t CRC_Return = 0;
uint8_t Sub_Dim_Return_Reg = 0;
uint8_t Return_Buf[MAX_SIZE];
uint8_t Return_Index = 0;
uint8_t Transmit_Data[MAX_SIZE];
uint8_t SIZE = 0;
uint16_t adc_value[2];
uint16_t DATA_IN[25];
uint16_t PWM_DUTY = 0;

// ----------------------- General Functions ------------------------------//

uint16_t MB_Start_ADD(uint8_t *Receive_Data)
{
	return (Receive_Data[3] | (Receive_Data[2] << 8));
}

uint16_t MB_Register_Count(uint8_t *Receive_Data)
{
	return (Receive_Data[5] | (Receive_Data[4] << 8));
}

void Clear_Return(uint8_t Return_Buffer[MAX_SIZE], uint8_t SIZE)
{
	for (int i = 0; i <= SIZE; i++)
	{
		Return_Buffer[i] = 0;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		Clear_Return(Return_Buf, MAX_SIZE);
	}
}

//-------------------------- END -------------------------------------//

//-------------------------- FUNCTION _3 / _4 -------------------------------------//

void Append_Data_Register(uint16_t Start_ADD, uint16_t Reg_Count, uint16_t Data_IN[16])
{
	for (uint8_t c = 0; c < Reg_Count; c++)
	{
		Return_Buf[Return_Index + 3] = (Data_IN[Start_ADD + c] >> 8);
		Return_Buf[Return_Index + 4] = (Data_IN[Start_ADD + c]);
		Return_Index += 2;
	}

	Return_Index = 0;
}

void Append_CRC_Register(uint8_t sub_size)
{
	CRC_Return = ModRTU_CRC(Return_Buf, sub_size);
	Return_Buf[sub_size] = CRC_Return;
	Return_Buf[sub_size + 1] = CRC_Return >> 8;

	CRC_Return = 0;
}

//	READ_MULTIPLE_HOLDING_REGISTER (_3)

void Read_Multiple_Holding_Register(uint8_t Receive_Data[MAX_SIZE], uint8_t len, uint8_t Device_Add, uint8_t xFunction)
{
	uint16_t Start_ADD = MB_Start_ADD(Receive_Data);
	uint16_t Reg_Count = MB_Register_Count(Receive_Data);

	Return_Buf[0] = Device_Add;
	Return_Buf[1] = xFunction;
	Return_Buf[2] = Reg_Count * 2;
	Append_Data_Register(Start_ADD, Reg_Count, DATA_IN);
	Append_CRC_Register(Reg_Count * 2 + 3);
	SIZE = Reg_Count * 2 + 5;
	HAL_UART_Transmit_IT(&huart1, Return_Buf, SIZE);

}

//	READ_INPUT_REGISTERS (_4)

void Read_Input_Registers(uint8_t Receive_Data[MAX_SIZE], uint8_t Device_Add,uint8_t xFunction)
{
	uint16_t Start_ADD = MB_Start_ADD(Receive_Data);
	uint16_t Reg_Count = 1;

	Return_Buf[0] = Device_Add;
	Return_Buf[1] = xFunction;
	Return_Buf[2] = Start_ADD;
	Return_Buf[3] = Reg_Count * 2;
	Append_Data_Register(Start_ADD, Reg_Count, DATA_IN);
	Append_CRC_Register(Reg_Count * 2 + 3);
	SIZE = Reg_Count * 2 + 6;
	HAL_UART_Transmit_IT(&huart1, Return_Buf, SIZE);
}

//------------------------------ END -----------------------------------------//

//----------------------------- FUNCTION _6 / _16 ----------------------------------//

void Write_Registers(uint16_t Start_ADD, uint16_t Reg_Count, uint16_t Data_IN[16], uint8_t Receive_Data[MAX_SIZE])
{
	for (uint8_t c = 0; c < Reg_Count; c++)
	{
		Data_IN[Start_ADD + c] = (Receive_Data[8 + Return_Index] | (Receive_Data[7 + Return_Index] << 8));
		Return_Index += 2;

	}
	Return_Index = 0;
}

//	WRITE_SINGLE_HOLDING_REGISTER ( _6 )

void Write_Single_Register(uint8_t Receive_Data[MAX_SIZE], uint8_t Device_Add,uint8_t xFunction)
{
	uint16_t Start_ADD = MB_Start_ADD(Receive_Data);
	uint16_t Output = MB_Register_Count(Receive_Data);

	Return_Buf[0] = Device_Add;
	Return_Buf[1] = xFunction;
	Return_Buf[2] = Start_ADD;
	Return_Buf[3] = 1;
	DATA_IN[Start_ADD] = Output;
	Append_CRC_Register(4);
	SIZE = 6;
	HAL_UART_Transmit_IT(&huart1, Return_Buf, SIZE);

}

//	WRITE_MULTIPLE_HOLDING_REGISTER ( _16 )

void Write_Multiple_Holding_Registers(uint8_t Receive_Data[MAX_SIZE],uint8_t len, uint8_t Device_Add, uint8_t xFunction)
{
	uint16_t Start_ADD = MB_Start_ADD(Receive_Data);
	uint16_t Reg_Count = MB_Register_Count(Receive_Data);

	Return_Buf[0] = Device_Add;
	Return_Buf[1] = Receive_Data[1];
	Return_Buf[2] = Start_ADD;
	Return_Buf[3] = Reg_Count;

	Write_Registers(Start_ADD, Reg_Count, DATA_IN, Receive_Data);
	Append_CRC_Register(4);
	SIZE = 6;
	HAL_UART_Transmit_IT(&huart1, Return_Buf, SIZE);

}

//------------------------------ END -----------------------------------------//

//--------------------------------- DATA ADC - DAC -----------------------------------------//

float Vadc_1, Vadc_2;
uint32_t Vcal_adc = 307700;
float Cal_1000x = 1000;
uint16_t Dac_1, Dac_2;

void Data_ADC()
{
	DATA_IN[16] = Cal_1000x;
	HAL_ADC_Start_DMA(&hadc, (uint32_t*) adc_value, 2);
	Vadc_1 = 3.3 * adc_value[0] / (4095);
	Vadc_2 = 3.3 * adc_value[1] / (4095);

	DATA_IN[0] = Vadc_1 * Vcal_adc / DATA_IN[16];
	DATA_IN[1] = Vadc_2 * Vcal_adc / DATA_IN[16];
}

void Data_DAC()
{
	if (DATA_IN[8] < 3001)
	{
		Dac_1 = DATA_IN[8] * 4095 / 3 / DATA_IN[16];
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, Dac_1);
	}
	if (DATA_IN[8] > 3000)
	{
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 4095);
	}

	if (DATA_IN[9] < 3001)
	{
		Dac_2 = DATA_IN[9] * 4095 / 3 / DATA_IN[16];
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, Dac_2);
	}
	if (DATA_IN[9] > 3000)
	{
		HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, 4095);
	}

}

//------------------------------ END -----------------------------------------//

//--------------------------------- GPIO - OUTPUT -----------------------------------------//

void GPIO_OUTPUT()
{
	if (DATA_IN[10] == 1)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	else if (DATA_IN[10] == 0)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
		HAL_Delay(100);
	}
	if (DATA_IN[11] == 1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	else if (DATA_IN[11] == 0)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_Delay(100);
	}
	if (DATA_IN[12] == 1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	else if (DATA_IN[12] == 0)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_Delay(100);
	}
	if (DATA_IN[13] == 1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	else if (DATA_IN[13] == 0)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
		HAL_Delay(100);
	}
	if (DATA_IN[14] == 1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	else if (DATA_IN[14] == 0)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
		HAL_Delay(100);
	}
	if (DATA_IN[15] == 1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	else if (DATA_IN[15] == 0)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET);
		HAL_Delay(100);
	}

}

//------------------------------ END -----------------------------------------//

//--------------------------------- GPIO - INPUT -----------------------------------------//

void GPIO_INPUT()
{
	DATA_IN[2] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);
	DATA_IN[3] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1);
	DATA_IN[4] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2);
	DATA_IN[5] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3);
	DATA_IN[6] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4);
	DATA_IN[7] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5);

}

//------------------------------ END -----------------------------------------//

//--------------------------------- PWM -----------------------------------------//

void PWM_Initialize()
{
	DATA_IN[17] = 600 - 1;
	DATA_IN[18] = 47;
	DATA_IN[19] = 50;
	DATA_IN[20] = 75;
	// Timer 3 PWM signal
	TIM1->PSC = (DATA_IN[17]);
	TIM1->CCR1 = DATA_IN[19];
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

	// Timer 2 PWM signal
	TIM2->PSC = (DATA_IN[18]);
	TIM2->CCR1 = DATA_IN[20];
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

}

//------------------------------ END -----------------------------------------//

