#include "temp.h"

static uint8_t buf[16];   // An array of 8-bit elements for transmitting and receiving data
static float temp_c;      // Float variable for the real result in Celsius

// Function to process and output the temperature result received over SPI
void i2c_processResult(uint8_t *val, UART_HandleTypeDef *huart2)
{
    // Reconstruct the 12-bit raw temperature value from the two SPI bytes
    // Master sent: val[0] = rawValue >> 4, val[1] = (rawValue & 0xF) << 4
    int16_t rawValue = ((int16_t)(val[0] << 8) | (val[1] & 0xF0));
    rawValue >>= 4;  // Right shift to align the data correctly as a 12-bit integer

    // Check and handle 2's complement for negative temperature values
    if (rawValue & 0x0800)  // Check the sign bit (11th bit of a 12-bit integer)
    {
        rawValue |= 0xF000;  // Perform sign extension for negative values
    }

    // Convert raw value to Celsius (0.0625 degrees per bit)
    temp_c = rawValue * 0.0625;

    // Use absolute value so whole/frac splitting works for negatives too
    float abs_c      = temp_c < 0 ? -temp_c : temp_c;
    unsigned int temp_whole = (unsigned int)(abs_c);
    unsigned int temp_frac  = (unsigned int)((abs_c - temp_whole) * 100);
    const char *sign = temp_c < 0 ? "-" : "";

    sprintf((char*)buf, "%s%u.%02u C\n", sign, temp_whole, temp_frac);

    // Send the temperature result over UART
    HAL_UART_Transmit(huart2, buf, strlen((char*)buf), HAL_MAX_DELAY);
}
