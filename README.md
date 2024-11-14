 commit :  VHDL code : removing_CRC , Stm32 code : lidar_fpga_all_combined uart to tsest
frequency clock 320MHz always check the frequency and baud rate for the uart code

 No CRC , i am sending 16 bit, 8 bit good data and 8 bit garbage,(garbage is the  &&                HAL_SPI_Transmit(&hspi1, &crcToSend, 1, HAL_MAX_DELAY) == HAL_OK
 it is garbage becayse i removed CRC , bs if i dont send 8 bits , counter 3am yzid 2 by 2 

 lech jumping 2 by 2 ? is it because the lidar is comfiguring the SPI bus to 16 bit and i am using the same bus (but didn't you test it before adding the lidar? if yes then you are with 8 )
check why the FPGA is skipping a byte 

Implement the CRC,
Clean the code 
 
