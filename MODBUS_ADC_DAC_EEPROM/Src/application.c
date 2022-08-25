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


uint8_t Rx_Stop_Flag 						= OFF;
uint8_t Rx_Ready_Flag 						=  ON;
uint8_t Communication_Error_Flag			= OFF;

uint8_t Device_Add				 			=   11;
uint8_t Rx_Index		 		 			=   0;
uint8_t Timeout_Counter 		 		   	=   0;


uint8_t Modbus_Rtu_Receive		  	   [MAX_SIZE];
uint8_t Rx_Buffer				  	   [MAX_SIZE];
uint8_t Rx_TemBuffer						  [10];

uint8_t Receive_CRC  						  [2];
uint16_t CRC_Kontrol			     		=   0;
uint16_t CRC_Receive_Kontrol		 		=   0;
uint8_t BUFFER_SIZE 						=   0;



//-------------- Compute the MODBUS RTU CRC -------------------------------//

uint16_t ModRTU_CRC(uint8_t buf[MAX_SIZE], int len)
{
  uint16_t crc = 0xFFFF;

  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return crc;
}

//--------------------------------------- END -----------------------------------------//

//--------------------------------------- Receive -------------------------------------//

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
	  if(Rx_Ready_Flag == ON)
	  		  {
	  			  Timeout_Counter = 0;
	  		  	  HAL_TIM_Base_Start_IT(&htim6);
	  		  	  FirstReceive();
	  		  	  Modbus_Rtu_Receive[Rx_Index] = Rx_TemBuffer[0];
	  		  	  ++ Rx_Index;
	  	          BUFFER_SIZE = Rx_Index;
	  		  	  if(Rx_Index > 255)
	  		  	  {
	  		  		  Communication_Error_Flag = ON;
	  		  	  }
	  		  }
  }
}



void FirstReceive()
{
	HAL_UART_Receive_IT(&huart1, Rx_TemBuffer, 1);
}
//--------------------------------------- TIMER -------------------------------------//


int Rx_Stop_CTRL()
{
	if (Timeout_Counter > 32)
	{
		Rx_Stop_Flag 			= ON ;
		Rx_Ready_Flag 			= OFF;
		return 1;
	}
	return 0;

}

uint16_t count_TIM7 ;
uint16_t data_2 ;
// Timer interrupta girince çağrılan fonk.
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM6)
	{
		if (Timeout_Counter < 40 )
		{
			Timeout_Counter++;
		}

	}
}



//--------------------------------------- END -----------------------------------------//


//--------------------------------------- MAIN -------------------------------------//


void General_Main()
{
	if ( Rx_Stop_Flag == ON  )
	{
		CRC_Receive_Kontrol 	= ModRTU_CRC(Modbus_Rtu_Receive,BUFFER_SIZE-2);
		Receive_CRC[0] 			= CRC_Receive_Kontrol & 0xff				  ;
		Receive_CRC[1] 			= (CRC_Receive_Kontrol >> 8 ) & 0xff		  ;

		if (Receive_CRC[0] == Modbus_Rtu_Receive[BUFFER_SIZE-2] && Receive_CRC[1] == Modbus_Rtu_Receive[BUFFER_SIZE-1] )
		{
			// Aygıt adresi kontrolü
			if (Device_Add == Modbus_Rtu_Receive[0])
			{
				// Fonksiyon işlemleri

				switch(Modbus_Rtu_Receive[1])
					{
//					//	READ_COILS
//					case 1:
//						break;
//					//	READ_DISCRETE_INPUTS
//					case 2:
//						break;
					//	READ_MULTIPLE_HOLDING_REGISTER
					case 3:
						Read_Multiple_Holding_Register(Modbus_Rtu_Receive,BUFFER_SIZE,Device_Add,Modbus_Rtu_Receive[1]);
						Clear_Receive(Modbus_Rtu_Receive,BUFFER_SIZE);
						Clear_Flags();
						break;
					//	READ_INPUT_REGISTERS
					case 4:
						Read_Input_Registers(Modbus_Rtu_Receive,Device_Add,Modbus_Rtu_Receive[1]);
						Clear_Receive(Modbus_Rtu_Receive,BUFFER_SIZE);
						Clear_Flags();
						break;
					//	WRITE_SINGLE_COIL
//					case 5:
//						//Write_Single_Coil(Modbus_Rtu_Receive, BUFFER_SIZE,Device_Add,xFunction);
//							break;
					//	WRITE_SINGLE_HOLDING_REGISTER
					case 6:
						Write_Single_Register(Modbus_Rtu_Receive,Device_Add,Modbus_Rtu_Receive[1]);
						Clear_Receive(Modbus_Rtu_Receive,BUFFER_SIZE);
						Clear_Flags();
						break;
//					//	WRITE_MULTIPLE_COILS
//					case 15:
//							break;
					//	WRITE_MULTIPLE_HOLDING_REGISTER
					case 16:
						Write_Multiple_Holding_Registers(Modbus_Rtu_Receive, BUFFER_SIZE,Device_Add,Modbus_Rtu_Receive[1]);
						Clear_Receive(Modbus_Rtu_Receive,BUFFER_SIZE);
						Clear_Flags();
						break;
					default:
						Clear_Flags();
						Clear_Receive(Modbus_Rtu_Receive,MAX_SIZE);
						break;
					}
			}
			else
			{
				Communication_Error_Flag 	=  ON;
				Clear_Flags();
				Clear_Receive(Modbus_Rtu_Receive,MAX_SIZE);
			}

		}
		else
		{
			Communication_Error_Flag 	=  ON;
			Clear_Flags();
			Clear_Receive(Modbus_Rtu_Receive,MAX_SIZE);
		}
		Clear_Flags();
	}

 }

//--------------------------------------- END -----------------------------------------//

//------------------------------ CLEANING -----------------------------------------//

void Clear_Flags()
{
	Rx_Ready_Flag 				=  ON;
	Rx_Stop_Flag				= OFF;
	Rx_Index 					=   0;
	Timeout_Counter             =   0;
	BUFFER_SIZE                 =   0;
	Communication_Error_Flag	= OFF;
}

void Clear_Receive( uint8_t MB_Receive[MAX_SIZE],uint8_t SIZE)
{
	for (int i = 0 ; i < SIZE ;i++)
	{
		MB_Receive[i] = 0;
	}

}

//--------------------------------------- END -----------------------------------------//
