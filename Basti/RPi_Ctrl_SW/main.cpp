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

int main(int argc, char **argv)
{
	uint32_t spi_clock_speed = 500000;
	double rhd2000sampling_rate = 5000;
	Data_Logger data_logger;
	data_logger.data_logging(spi_clock_speed, rhd2000sampling_rate);
	return 0;
}

