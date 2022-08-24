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
#include <time.h>
#include <chrono>
#include <thread>
#include <iomanip>

#include "data_logger.h"
#include "bcm2835_chip.h"
#include "rhd2000registers.h"

using namespace std;
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

// Constructor: initialise the class
Data_Logger::Data_Logger()
{

}

// Destructor: close the class
Data_Logger::~Data_Logger()
{

}

// Function: Convert hexadecimal value from ADC to actual voltage value
float Data_Logger::adc_hex2float(uint16_t adc_value)
{
	//The LSB of the ADC is 0.195 uV and the circuit has a built-in offset of 1.225V which correspons to an adc value of 32754
	float voltage_result = ((float)(adc_value - 32754)) * 0.195;
	return voltage_result;
}

// Function: Save adc results of specified channel to file (append measurements to existing file)
void Data_Logger::save_measurements_to_file(vector<uint16_t>& u16_vector, int timestamp, int channel)
{
	FILE * pFile;
	pFile = fopen ("measurements.dat","a");
	if (pFile != NULL)
	{
		//the adc result is delayed by two spi transfers. Therefore we have to access the result vector at position channel+2
		fprintf(pFile, "%f %f\n", (float)timestamp, adc_hex2float(u16_vector[channel+2]));
		fclose (pFile);
	}
	else
	{
		printf("Error while opening the file! \n");
	}
}


// Function: acquires adc results of all 16 channels from rhd2000 chip. 
// Prints out values of each channel and saves values of the specified channel in a file
void Data_Logger::data_logging(uint32_t spi_clock_speed, double rhd2000_sampleRate, int channel)
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

	//Initialize time variable to associate a time to each measurement of the specified channel
	clock_t StartTime = clock();
	clock_t timestamp = 0;
	
	//Initialize and calibrate RHD2000
	VectorLength = rhd2000regs.createCommandListRegisterConfig(rhd2000commandVector, true);
	//Perform SPI Transfer, let received half words be stored in rhd2000receivedResVector
	bcm2835_board.spi_transfer(rhd2000commandVector, rhd2000receivedResVector);
	//Create command vector to let the rhd2000 send out adc results of all 16 channels
	VectorLength = rhd2000regs.createCommandListConvert(rhd2000commandVector);
	//endless loop that collects ADC results regularly
	while(true)
	{
		//Make sure result vector is empty and can store new results
		rhd2000receivedResVector.clear();
		//Perform SPI transfer, let received half words be stored in rhd2000receivedResVector
		bcm2835_board.spi_transfer(rhd2000commandVector, rhd2000receivedResVector);
		timestamp = clock() - StartTime;
		//Print ADC results always to the same line
		for(int k = 0; k <= 15; k++)
		{
			cout << "C" << k << ":" << adc_hex2float(rhd2000receivedResVector[k+2]) << " ";
		}
		cout << "\r" << flush;
		//save adc results in a file
		save_measurements_to_file(rhd2000receivedResVector, timestamp, channel);
	}
}
