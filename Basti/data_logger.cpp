//----------------------------------------------------------------------------------
// Copyright 2022 Universitaetsklinikum Freiburg
// Section of Neuroelectronic Systems
// Authors: Bastian Harder
// Project: CIAM
// File Description: This class provides functions to read data from INTAN RHD2000 
// over spi and save this data in a file. 
// This class makes use of  bcm2835_chip class and rhd2000registers class. 
//----------------------------------------------------------------------------------

#include "data_logger.h"
#include "bcm2835_chip.h"
#include "rhd2000registers.h"

#include <stdint.h>
#include <stdio.h>

using namespace std;

// Constructor: initialise the class
Data_Logger::Data_Logger()
{

}

// Destructor: close the class and deallocate any memory
Data_Logger::~Data_Logger()
{

}


// Function:
void Data_Logger::data_logging(uint32_t spi_clock_speed, double rhd2000_sampleRate)
{
	//generate Objects of classes
	BCM2835_Chip bcm2835_board;
	Rhd2000Registers rhd2000regs(rhd2000_sampleRate);
	
	//generate vector object to store commands for rhd2000
	vector<uint16_t> rhd2000commandVector;
	//generate vector object to store received results from rhd2000
	vector<uint16_t> rhd2000receivedResVector;
	//variable to store length of command vector
	int VectorLength = 0;
	
	//Initialize SPI core
	bcm2835_board.spi_init(spi_clock_speed);
	
	//Initialize and calibrate RHD2000
	VectorLength = rhd2000regs.createCommandListRegisterConfig(rhd2000commandVector, true);
	//Print whole command vector for checking purpose
	printf("Command Vector Content: \n");
	for(int i = 0; i < VectorLength; i++)
	{
		printf("Vector Index %d: \t %x \n", i, rhd2000commandVector[i]);
	}
	
	bcm2835_board.spi_transfer(rhd2000commandVector, rhd2000receivedResVector);	
	VectorLength = static_cast<int>(rhd2000receivedResVector.size());
	//Print whole result vector for checking purpose
	printf("Result Vector Content: \n");
	printf("Result Vector Length: %d \n", VectorLength);
	for(int k = 0; k < VectorLength; k++)
	{
		printf("Vector Index %d: \t %x \n", k, rhd2000receivedResVector[k]);
	}
	
	//Read ADC results from all 16 channels
	//rhd2000regs.createCommandListConvert(rhd2000commandVector);
	//rhd2000receivedResVector.clear();
	//bcm2835_board.spi_transfer(rhd2000commandVector, rhd2000receivedResVector);
	
}



