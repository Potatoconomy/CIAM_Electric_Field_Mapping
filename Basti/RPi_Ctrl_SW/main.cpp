//----------------------------------------------------------------------------------
// Copyright 2022 Universitaetsklinikum Freiburg
// Section of Neuroelectronic Systems
// Authors: Bastian Harder
// Project: CIAM
// File Description: main function
//----------------------------------------------------------------------------------

#include <stdio.h>
#include "data_logger.h"
#include <stdint.h>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
	uint32_t spi_clock_speed = 16000000;		//Set SPI clock to 16MHz
	double rhd2000sampling_rate = 800000;		//As one SPI transfer needs roughly 20 clock cyclces the overall sampling rate is spi_clock_speed/20
	//Take the channel from which the data shall be plotted from the command line arguments
	int input_argument;
	int channel_for_plotting = 4;	//default channel
	cout << "Default Channel: " << channel_for_plotting << endl;
	//check that there is only one command line argument and that 
	if(argc == 2)
	{
		//convert input to integer
		input_argument = stoi(argv[1]);
		//check that input is between 0 and 15 (only 16 Channels)
		if((input_argument >= 0) && (input_argument <= 15))
		{
			channel_for_plotting = input_argument;
			cout << "Logging Channel " << channel_for_plotting << endl;
		}
		else
		{
			cout << "Please specifiy a channel between 0 and 15!" << endl;
			cout << "Usage example for Channel 4: sudo ./rhd2000acquisitionmain 4" << endl;
		}
	}
	else
	{
		cout << "Too many or bad command line arguments!" << endl;
		cout << "Usage example for Channel 4: sudo ./rhd2000acquisitionmain 4" << endl;
	}
	//Create instance of class data logger and start logging the specified channel
	Data_Logger data_logger;
	data_logger.data_logging(spi_clock_speed, rhd2000sampling_rate, channel_for_plotting);
	return 0;
}
