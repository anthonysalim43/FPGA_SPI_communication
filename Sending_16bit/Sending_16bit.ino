#include "stm32f4xx_hal.h"  // Ensure you include the right header file for your MCU family

#define SerialPort Serial
uint8_t dataToSend = 0xCF;


//uint8_t array_dataToSend[] = {0xAA, 0xCD,0XFF,0XDE,0X35};
uint16_t array_dataToSend[] = {0xFFAA};
uint8_t byte_to_send;
//uint8_t array_dataToSend[] = {170,205 ,255,222,53};
uint8_t number_byte_to_send=0;



uint8_t receivedData = 0x00;  // Make receivedData a global variable
uint8_t crcToSend;  // Global variable to store 8-bit CRC

uint16_t KB_dataToSend[100];
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
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;//for Hex data 1EDGE, for DEC 2EDGE
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

void SPI_data_Send(uint8_t *data, uint8_t length) {

  //  if ((digitalRead(PC13) == GPIO_PIN_RESET) && pressed_once == 0) {
        counter_crc=0;
        pressed_once = 1;
        uint8_t received_correctly_0, received_correctly_1;

       
        do {
            counter_crc++;
            crcToSend = calculateCRC8(data, length);  // Calculate CRC over the data

            if (hspi1.State == HAL_SPI_STATE_BUSY) {
                SerialPort.println("Aborting previous SPI transmission");
                if (HAL_SPI_Abort(&hspi1) != HAL_OK) {
                    SerialPort.println("Error aborting SPI");
                }
            }
             // Select the slave
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
            SerialPort.println("Starting SPI Transmission");
            // Transmit data
            if (HAL_SPI_Transmit(&hspi1, data, length, HAL_MAX_DELAY) == HAL_OK) {//length is how many byte you are going to send 
                                                                                      //data is the array of data 
                SerialPort.println("Data Transmission Complete");
                // Transmit CRC
                if (HAL_SPI_Transmit(&hspi1, &crcToSend, 1, HAL_MAX_DELAY) != HAL_OK) {
                    SerialPort.println("Error in CRC Transmission");
                }
            } else {
                SerialPort.println("Error in SPI Transmission");
                SerialPort.print("SPI Error Code: ");
                SerialPort.println(hspi1.ErrorCode, HEX);
            }

            // Check response from FPGA
            received_correctly_0 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_13);
            received_correctly_1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_14);
            SerialPort.println(crcToSend, HEX);
            SerialPort.print("CRC counter =");
            SerialPort.println(counter_crc,DEC);
        } while (!(received_correctly_0 == GPIO_PIN_RESET && received_correctly_1 == GPIO_PIN_RESET));// Check for '00' status

        // Deselect the slave
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        SerialPort.println("Transmission Done");

        // Reset button press flag to allow re-triggering
        pressed_once = 0;
  //  }
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




    //Creating 1KB of data

    for (int i=0; i< 100;i++){

        KB_dataToSend[i]=65535-i;
        SerialPort.println( KB_dataToSend[i],DEC);

}

}

void loop()
{

/*

SerialPort.println("Start");

uint8_t array_dataToSend[] = {170,205 ,255,222,53};

//uint8_t array_dataToSend[] = {0xAA, 0xCD,0XFF,0XDE,0X35};
for (int i=0; i< 5;i++){

        SerialPort.println();
        SerialPort.print("DEC=");
         SerialPort.print(array_dataToSend[i],DEC);

        SerialPort.print("HEX=");
         SerialPort.print(array_dataToSend[i],HEX);

}
uint16_t byte4=0xABCD;

uint8_t byte2=byte4>>8 ;
uint8_t byte1=byte4 ;

SerialPort.println("byteMSB=");
SerialPort.print(byte2,HEX);

SerialPort.println("byteLSB=");
SerialPort.print(byte1,HEX);
*/

 if ((digitalRead(PC13) == GPIO_PIN_RESET) && pressed_once == 0) {
    number_byte_to_send = sizeof(KB_dataToSend)/sizeof(KB_dataToSend[0]);
    for (int i=0; i< number_byte_to_send;i++){

     /*  if(sizeof(array_dataToSend[i])==1){

          SPI_data_Send(&array_dataToSend[i], sizeof(array_dataToSend[i]));//1 byte 

        }

        else{//2 bytes*/
          byte_to_send=KB_dataToSend[i]>>8;
          SPI_data_Send(&byte_to_send, sizeof(byte_to_send));
          byte_to_send=KB_dataToSend[i];
          SPI_data_Send(&byte_to_send, sizeof(byte_to_send));//it is 2 byte since it is 16 bit but i am sending 8 bit at once 
        //}
         

    }

 }

 /*if ((digitalRead(PC13) == GPIO_PIN_RESET) && pressed_once == 0) {
    number_byte_to_send = sizeof(array_dataToSend)/sizeof(array_dataToSend[0]);
    for (int i=0; i< number_byte_to_send;i++){

     //  if(sizeof(array_dataToSend[i])==1){

       //   SPI_data_Send(&array_dataToSend[i], sizeof(array_dataToSend[i]));//1 byte 

        //}

        //else{//2 bytes
          byte_to_send=array_dataToSend[i]>>8;
          SPI_data_Send(&byte_to_send, sizeof(byte_to_send));
          byte_to_send=array_dataToSend[i];
          SPI_data_Send(&byte_to_send, sizeof(byte_to_send));//it is 2 byte since it is 16 bit but i am sending 8 bit at once 
        //}
         

    }

 }*/
}
