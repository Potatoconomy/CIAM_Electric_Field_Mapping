//----------------------------------------------------------------------------------
// Copyright 2022 UniversitĂ¤tsklinikum Freiburg
// Section of Neuroelectronic Systems
// Authors: Bastian Harder
// Project: CIAM
// File Description: This class provides several board functions e.g. SPI, 
// using the C library for bcm2835 from https://www.airspayce.com/mikem/bcm2835/
//----------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <bcm2835.h>
#include "bcm2835_chip.h"

using namespace std;

// Constructor: initialises the bcm2835 library
BCM2835_Chip::BCM2835_Chip()
{
	if(bcm2835_init() == FAIL)
	{
		printf("bcm2835_init failed! Are you running as root? \n");
	}
	else
	{
		printf("Initialized BCM2835 \n");
	}
}

// Destructor: close the bcm2835 library and deallocate any memory
// close spi dev
BCM2835_Chip::~BCM2835_Chip()
{
	bcm2835_spi_end();
	printf("Closed SPI Dev \n");
	if(bcm2835_close() == FAIL)
	{
		printf("bcm2835_close failed! Are you running as root? \n");
	}
	else
	{
		printf("Closed BCM2835 \n");
	}
}

//Function: Initialize bcm2835 spi controller with the specified frequency and Mode for operation with INTAN RHD2000
void BCM2835_Chip::spi_init(uint32_t spi_speed_hz)
{
	if(bcm2835_spi_begin() == FAIL)
	{
		printf("bcm2835_spi_begin failed! Are you running as root? \n");
	}
	else
	{
		bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
		bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);  // Different from Intan chip setting
		bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
		bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
		bcm2835_spi_set_speed_hz(spi_speed_hz);
		// write clock speed to member variable
		_spi_clock_hz = spi_speed_hz;
		printf("Initialized SPI with spi clock speed: %d \n", _spi_clock_hz);
	}	
}

//Function: Get value of SPI clock speed
uint32_t BCM2835_Chip::spi_get_clock_freq()
{
	return _spi_clock_hz;
}

//Function: send 8bit value and simultanuously receive 8bit over spi
uint8_t BCM2835_Chip::spi_transfer8(uint8_t value)
{
	return bcm2835_spi_transfer(value);
}

//Function: send (and receive) 16bit value over spi
void BCM2835_Chip::spi_transfer16(char* tbuf, char* rbuf)
{	
	bcm2835_spi_transfernb(tbuf, rbuf, 2);
}

//Function: Take a commandList, send the commands over spi, simultanuously save the received answers from slave in receivedList 
//This function may be optimised by using pointers instead of copying the whole vector to a char List!
//Maybe use something like this: char* tbuf = <reinterpret_cast><char*>comandList;
void BCM2835_Chip::spi_transfer(vector<uint16_t> &commandList, vector<uint16_t> &receivedList)
{
	int num_of_commands = static_cast<int>(commandList.size());
	
	char* transmit_buffer = new char[2];
	char* receive_buffer = new char[2*num_of_commands];
	char* p_rbuf = receive_buffer;   //pointer which counts up in the following for loop to let the received bytes be written to the whole receive buffer
	uint16_t received_halfword16 = 0x0000;
		
	for(int i = 0; i < num_of_commands; i++)
	{
		transmit_buffer[1] = (char) (commandList[i] & 0x00ff);
		transmit_buffer[0] = (char) (commandList[i] >> 8);
		spi_transfer16(transmit_buffer, p_rbuf);
		received_halfword16 = ((uint16_t)p_rbuf[0] << 8) + ((uint16_t)p_rbuf[1]);
		receivedList.push_back(received_halfword16);
		//increment p_rbuf in case the received halfword was not the last one
		if(i < (num_of_commands-1)) {p_rbuf += 2;}
	}
}
