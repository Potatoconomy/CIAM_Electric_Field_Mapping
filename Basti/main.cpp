// blink.c
//
// Example program for bcm2835 library
// Blinks a pin on an off every 0.5 secs
//
// After installing bcm2835, you can build this 
// with something like:
// gcc -o blink blink.c -l bcm2835
// sudo ./blink
//
// Or you can test it before installing with:
// gcc -o blink -I ../../src ../../src/bcm2835.c blink.c
// sudo ./blink
//
// Author: Mike McCauley
// Copyright (C) 2011 Mike McCauley
// $Id: RF22.h,v 1.21 2012/05/30 01:51:25 mikem Exp $

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

