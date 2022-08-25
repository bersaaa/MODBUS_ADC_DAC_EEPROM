#ifndef INC_MODBUS_FUNCTION_H_
#define INC_MODBUS_FUNCTION_H_


uint16_t MB_Start_ADD(uint8_t *Receive_Data);
uint16_t MB_Register_Count(uint8_t *Receive_Data);
void Clear_Return(uint8_t Return_Buffer[256],uint8_t SIZE);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void Append_Data_Register(uint16_t Start_ADD , uint16_t Reg_Count , uint16_t Data_IN[16]);
void Append_CRC_Register(uint8_t sub_size);
void Read_Multiple_Holding_Register(uint8_t Receive_Data[256], uint8_t len,uint8_t Device_Add,uint8_t xFunction);
void Read_Input_Registers(uint8_t Receive_Data[256],uint8_t Device_Add,uint8_t xFunction);
void Write_Registers(uint16_t Start_ADD , uint16_t Reg_Count , uint16_t Data_IN[16],uint8_t Receive_Data[256]);
void Write_Multiple_Holding_Registers(uint8_t Receive_Data[256], uint8_t len,uint8_t Device_Add,uint8_t xFunction);
void Write_Single_Register(uint8_t Receive_Data[256],uint8_t Device_Add,uint8_t xFunction);
void Data_ADC();
void Data_DAC();
void GPIO_OUTPUT();
void GPIO_INPUT();
void PWM_Initialize();

#endif /* INC_MODBUS_FUNCTION_H_ */
