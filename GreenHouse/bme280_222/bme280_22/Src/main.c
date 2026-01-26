#include "stm32f4xx.h"
#include "bme280.h"
#include "i2c.h"
#include "uart.h"
#include <stdint.h>
#include <stdio.h>

void delay(volatile uint32_t count)
{
    while (count--);
}

int main(void)
{
    float temperature;
    float humidity;

    /* Initialize I2C for BME280 */
    I2CInit();

    /* Initialize UART */
    UartInit(BAUD_115200);
    UartPuts("UART Init OK\r\n");

    /* Initialize BME280 */
    if (BME280_Init() != 0)
    {
        UartPuts("BME280 Init Failed!\r\n");
        while (1);
    }
    UartPuts("BME280 Init Successful\r\n");

    while (1)
    {
        /* Read temperature and humidity only */
        BME280_ReadTempHum(&temperature, &humidity);

        /* Print values via UART */
        char buf[64];
        sprintf(buf,
                "Temperature: %.2f C, Humidity: %.2f %%\r\n",
                temperature, humidity);
        UartPuts(buf);

        delay(500000);
    }
}
