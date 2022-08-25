#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_


// --------------- Functions -----------------------//
uint16_t ModRTU_CRC(uint8_t buf[8], int len);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void FirstReceive();
int Rx_Stop_CTRL();
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void General_Main();
void Clear_Flags();
void Clear_Receive( uint8_t MB_Receive[256],uint8_t MAX_SIZE);

#endif /* INC_APPLICATION_H_ */
