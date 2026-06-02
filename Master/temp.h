#ifndef INC_TEMP_H_
#define INC_TEMP_H_

#include <stm32f4xx_hal.h>
#include <string.h>  // For string operations (strcpy, sprintf)
#include <stdio.h>   // For formatted output (sprintf)

#define TMP102_ADDR (0x48 << 1)  // Use 8-bit address shifted by 1 for the Read/Write bit
#define REG_TEMP    0x00         // The register address for temperature reading

void    i2c_processResult(int16_t val, UART_HandleTypeDef *huart2);
int16_t i2c_communicate(I2C_HandleTypeDef *hi2c1, UART_HandleTypeDef *huart2);

#endif /* INC_TEMP_H_ */
