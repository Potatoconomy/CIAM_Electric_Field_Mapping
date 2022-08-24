//----------------------------------------------------------------------------------
// Copyright 2022 Universitaetsklinikum Freiburg
// Section of Neuroelectronic Systems
// Authors: Bastian Harder
// Project: CIAM
// File Description: This class provides functions to read data from INTAN RHD2000 
// over spi and save this data in a file. 
// This class makes use of  bcm2835_chip class and rhd2000registers class. 
//----------------------------------------------------------------------------------

#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <stdint.h>
#include <vector>

using namespace std;

class Data_Logger
{
public:
    Data_Logger();
	~Data_Logger();
	float adc_hex2float(uint16_t adc_result);
	void save_measurements_to_file(vector<uint16_t>& u16_vector, int timestamp, int channel); 
	void data_logging(uint32_t spi_clock_speed, double sampleRate, int channel);

private:

};

#endif // DATA_LOGGER_H
