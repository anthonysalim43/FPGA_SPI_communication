#include "stm32f4xx_hal.h"  // Ensure you include the right header file for your MCU family

#define SerialPort Serial
uint8_t dataToSend = 0x36;
uint8_t receivedData = 0x00;  // Make receivedData a global variable
uint8_t crcToSend;  // Global variable to store 8-bit CRC

// Declare the SPI handle
SPI_HandleTypeDef hspi1;
int pressed_once = 0;
int counter_crc=0;
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
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_ENABLE;
    hspi1.Init.CRCPolynomial = 7;  // This setting won't affect anything since CRCCalculation is disabled

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

// Software implementation of CRC-8 calculation with polynomial 0x07
uint8_t calculateCRC8(const uint8_t *data, uint16_t length)
{
    uint8_t crc = 0x00;  // Initial value

    for (uint16_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)//if it is 0 then false if it other then then it is true
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;//if the MSB was not 0 we shift by 1, we add a 0 then we don't neet to do a XOR with the  divideur 
        }
    }
    return crc;
}



void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable the GPIOA clock
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure PA0 and PA1 as output
    GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Set as push-pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


void setup()
{
    SerialPort.begin(9600);
    while (!SerialPort) {} // Wait for serial to be ready
    SerialPort.println("STM32 SPI Ready");
    HAL_Init();
    Init_SPI1();
    GPIO_Init();

             // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13 ,GPIO_PIN_SET);
              //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14 ,GPIO_PIN_SET);

    pinMode(PA13, INPUT); // It is LOW when you push it
    pinMode(PA14, INPUT); // It is LOW when you push it
    pinMode(PC13, INPUT); // It is LOW when you push it
    digitalWrite(PA4, HIGH);
}

void loop()
{
     /* HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13 ,GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14 ,GPIO_PIN_SET);
      delay(500);

      SerialPort.println(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13));
      SerialPort.println(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_14));
        //received_correctly_1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_14);

      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13 ,GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14 ,GPIO_PIN_RESET);
      delay(500);



      SerialPort.println(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13));
      SerialPort.println(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_14));
*/

    if ((digitalRead(PC13) == GPIO_PIN_RESET) && pressed_once == 0)
    {

      counter_crc=0;
        pressed_once = 1;
        uint8_t received_correctly_0, received_correctly_1;
        
        do {
            counter_crc++;
              //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13 ,GPIO_PIN_SET);
              //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14 ,GPIO_PIN_SET);
            crcToSend = calculateCRC8(&dataToSend, 1);  // Calculate CRC over the data

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

            // Transmit data and CRC
            if (HAL_SPI_Transmit(&hspi1, &dataToSend, 1, HAL_MAX_DELAY) == HAL_OK 
            //&&                HAL_SPI_Transmit(&hspi1, &crcToSend, 1, HAL_MAX_DELAY) == HAL_OK
            )
            {
                SerialPort.println("Transmission Complete");
            }
            else
            {
                SerialPort.println("Error in SPI Transmission");
                SerialPort.print("SPI Error Code: ");
                SerialPort.println(hspi1.ErrorCode, HEX);
            }

            
            // Read the received correctly signals from FPGA
            received_correctly_0 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13);
            received_correctly_1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_14);
            SerialPort.println(crcToSend, HEX);
            SerialPort.println(counter_crc, DEC);


            //HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);



       /*     while ((received_correctly_0 == 1 && received_correctly_1 == 1)) {
                // Keep polling until the FPGA has processed the data
              
              //  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13 ,GPIO_PIN_RESET);
                // HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14 ,GPIO_PIN_RESET);



                received_correctly_0 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13);
                received_correctly_1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_14);
                //digitalWrite(PA14, LOW);
               //  SerialPort.println(counter_crc);
               //  SerialPort.println("Hasasasasa");

            //     SerialPort.println(received_correctly_0);
             //    SerialPort.println(received_correctly_1);

            }
*/
        //    SerialPort.println("Here");
        } while (!(received_correctly_0 == GPIO_PIN_RESET && received_correctly_1 == GPIO_PIN_RESET));  // Check for '00' status


  // Deselect the slave
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
             SerialPort.println("Transmission Done");
        // Reset button press flag to allow re-triggering
       pressed_once = 0;
    }

   // SerialPort.println("Checking if we are outside");
}
