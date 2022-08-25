#ifndef INC_MCP_H_
#define INC_MCP_H_



/* ADC */
void CS_ADC_SELECT();
void CS_ADC_DESELECT();
uint16_t MCP3208_DIN_DOUT(uint16_t Channel);
void MCP3208();

/* DAC */
void CS_DAC_SELECT();
void CS_DAC_DESELECT();
void MCP4911_Write();

#endif /* INC_MCP_H_ */
