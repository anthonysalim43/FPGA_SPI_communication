#include "stm32f4xx_hal.h"  // Ensure you include the right header file for your MCU family

#define SerialPort Serial
uint8_t dataToSend = 0x88;
uint8_t receivedData = 0x00;  // Make receivedData a global variable

// Declare the SPI handle
SPI_HandleTypeDef hspi1;
int pressed_once = 0;

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if(hspi->Instance == SPI1)
    {
        __HAL_RCC_SPI1_CLK_ENABLE();  // Enable SPI1 clock
        __HAL_RCC_GPIOA_CLK_ENABLE(); // Enable GPIOA clock

        // SPI1 SCK, MISO, MOSI pin configuration
        GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1; // SPI1 Alternate function AF5
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // SPI1 NSS pin configuration (if used in hardware-controlled mode)
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void Init_SPI1(void)
{
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;

    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        SerialPort.println("SPI Initialization Error");
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        SerialPort.println("SPI Error Callback");
        SerialPort.print("SPI Error Code: ");
        SerialPort.println(hspi->ErrorCode, HEX);
    }
}

void setup()
{
    SerialPort.begin(9600);
    while (!SerialPort) {} // Wait for serial to be ready
    SerialPort.println("STM32 SPI Ready");
 //digitalWrite(PA4, HIGH);
    HAL_Init();
    Init_SPI1();

    pinMode(PC13, INPUT); // IT is LOW when you push it
    digitalWrite(PA4, HIGH);
}

void loop()
{
    // If the button is pressed, start SPI transmission
    if ((digitalRead(PC13) == LOW) && pressed_once == 0)
    {
        pressed_once = 1;

        if (hspi1.State == HAL_SPI_STATE_BUSY)
        {
            SerialPort.println("Aborting previous SPI transmission");
            if (HAL_SPI_Abort(&hspi1) != HAL_OK)
            {
                SerialPort.println("Error aborting SPI");
            }
        }

        // Select the slave
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

        SerialPort.println("Starting SPI Transmission");

        // Check if SPI is ready
        if (hspi1.State == HAL_SPI_STATE_READY)
        {
            // Transmit data using polling (blocking) mode
            if (HAL_SPI_Transmit(&hspi1, &dataToSend, 1, HAL_MAX_DELAY) != HAL_OK)
            {
                SerialPort.println("Error in SPI Transmission");
                SerialPort.print("SPI Error Code: ");
                SerialPort.println(hspi1.ErrorCode, HEX);
            }
            else
            {


               // Deselect the slave
            
                SerialPort.println("Transmission Complete");
            }

           HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        }
        else
        {
            SerialPort.println("SPI not ready");
        }
    }

 //   NVIC_SystemReset();  // Perform a system reset
}
