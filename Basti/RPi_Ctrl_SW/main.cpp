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
	uint32_t spi_clock_speed = 16000000;
	double rhd2000sampling_rate = 800000;
	uint8_t channel_for_plotting = 4;
	Data_Logger data_logger;
	data_logger.data_logging(spi_clock_speed, rhd2000sampling_rate, channel_for_plotting);
	return 0;
}
