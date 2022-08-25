#ifndef INC_PROJECT_SPI_H_
#define INC_PROJECT_SPI_H_

void SELECT();
void DESELECT();
void SPI_Command(uint8_t cmd);

void Write_SEQ_u8(uint16_t addr, uint8_t data);
uint8_t Read_SEQ_u8(uint16_t addr);

void Write_SEQ_u16(uint16_t addr, uint16_t data);
uint16_t Read_SEQ_u16(uint16_t addr);

void Write_SEQ_u32(uint16_t addr, uint32_t data);
uint32_t Read_SEQ_u32(uint16_t addr);

void Process_Spi();



#endif /* INC_PROJECT_SPI_H_ */
