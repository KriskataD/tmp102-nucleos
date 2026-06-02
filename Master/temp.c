#include "temp.h"  // Include the header file for I2C functions
#include "FreeRTOS.h"
#include "semphr.h"

static HAL_StatusTypeDef ret;           // Used to check the status of the HAL functions
static uint8_t buf[16];                 // An array of 8-bit elements for transmitting and receiving data
static float temp_c;                    // Float variable for the real result in Celsius
static SemaphoreHandle_t rxSemaphore = NULL;

// DMA reception complete callback — called from interrupt context
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(rxSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Function to communicate with the temperature sensor over I2C
int16_t i2c_communicate(I2C_HandleTypeDef *hi2c1, UART_HandleTypeDef *huart2)
{
    if (rxSemaphore == NULL)
        rxSemaphore = xSemaphoreCreateBinary();

    // Initiate a non-blocking read of 2 bytes from the temperature register using DMA
    ret = HAL_I2C_Mem_Read_DMA(hi2c1, TMP102_ADDR, REG_TEMP, 1, buf, 2);

    if (ret != HAL_OK)  // If reception fails, send error message over UART
    {
        HAL_UART_Transmit(huart2,
                          (uint8_t*) "Error Rx\r\n",
                          10,
                          HAL_MAX_DELAY);
        return -1;  // Return error code
    }

    // Block the task (yielding the CPU) until the DMA callback gives the semaphore
    xSemaphoreTake(rxSemaphore, portMAX_DELAY);

    // Combine the two bytes into a 12-bit value (temperature)
    return ((int16_t) buf[0] << 4) | (buf[1] >> 4);
}

// Function to process and output the temperature result
void i2c_processResult(int16_t val, UART_HandleTypeDef *huart2)
{
    // Check and handle 2's complement for negative temperature values
    if (val > 0x7FF)
    {
        val |= 0xF000;  // Extend sign if temperature is negative
    }

    // Convert raw value to Celsius (0.0625 degrees per bit)
    temp_c = val * 0.0625;

    // Convert temperature to decimal format with two decimal places
    temp_c *= 100;  // Scale up for easier sprintf handling
    sprintf((char*) buf,
            "%u.%02u C\n",
            (unsigned int) temp_c / 100,
            temp_c % 100);

    // Send the temperature result over UART
    HAL_UART_Transmit(huart2, buf, strlen((char*) buf), HAL_MAX_DELAY);
}
