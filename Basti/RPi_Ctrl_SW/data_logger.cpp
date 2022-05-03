//----------------------------------------------------------------------------------
// Copyright 2022 Universitaetsklinikum Freiburg
// Section of Neuroelectronic Systems
// Authors: Bastian Harder
// Project: CIAM
// File Description: This class provides functions to read data from INTAN RHD2000 
// over spi and save this data in a file. 
// This class makes use of  bcm2835_chip class and rhd2000registers class. 
//----------------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "data_logger.h"
#include "bcm2835_chip.h"
#include "rhd2000registers.h"



using namespace std;

// Constructor: initialise the class
Data_Logger::Data_Logger()
{

}

// Destructor: close the class and deallocate any memory
Data_Logger::~Data_Logger()
{

}

// Function: Save Vector content in a file (append vector content to existing file)
void Data_Logger::save_vector_to_file(vector<uint16_t>& u16_vector)
{
	int VectorLength = static_cast<int>(u16_vector.size());
	FILE * pFile;

	pFile = fopen ("rhd2000_results.txt","a");
	if (pFile != NULL)
	{
		fprintf(pFile, "Result Vector content: \n");
		for (int i = 2; i < VectorLength; i++)
		{
			fprintf(pFile, "Result of ADC Channel %d: \t %x \n", (i-2), u16_vector[i]);
		}
		fclose (pFile);
	}
	else
	{
		printf("Error while opening the file! \n");
	}
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
	printf("Config Command Vector Content: \n");
	for(int i = 0; i < VectorLength; i++)
	{
		printf("Vector Index %d: \t %x \n", i, rhd2000commandVector[i]);
	}
	//Perform SPI Transfer, let received half words be stored in rhd2000receivedResVector
	bcm2835_board.spi_transfer(rhd2000commandVector, rhd2000receivedResVector);	
	VectorLength = static_cast<int>(rhd2000receivedResVector.size());
	//Print whole result vector for checking purpose
	printf("Config Result Vector Content: \n");
	printf("Result Vector Length: %d \n", VectorLength);
	for(int k = 0; k < VectorLength; k++)
	{
		printf("Vector Index %d: \t %x \n", k, rhd2000receivedResVector[k]);
	}
	
	//endless loop that collects ADC results regularly
	//while(true)
	//{
		//Create command vector to let the rhd2000 send out all adc results
		VectorLength = rhd2000regs.createCommandListConvert(rhd2000commandVector);
		//Print whole command vector for checking purpose
		printf("Convert Command Vector Content: \n");
		for(int i = 0; i < VectorLength; i++)
		{
			printf("Vector Index %d: \t %x \n", i, rhd2000commandVector[i]);
		}
		//Make sure result vector is empty and can store new results
		rhd2000receivedResVector.clear();
		//Perform SPI transfer, let received half words be stored in rhd2000receivedResVector
		bcm2835_board.spi_transfer(rhd2000commandVector, rhd2000receivedResVector);
		VectorLength = static_cast<int>(rhd2000receivedResVector.size());
		//Print whole result vector for checking purpose
		printf("Convert Result Vector Content: \n");
		printf("Result Vector Length: %d \n", VectorLength);
		for(int k = 0; k < VectorLength; k++)
		{
			printf("Vector Index %d: \t %x \n", k, rhd2000receivedResVector[k]);
		}
		save_vector_to_file(rhd2000receivedResVector);
	//}
}



