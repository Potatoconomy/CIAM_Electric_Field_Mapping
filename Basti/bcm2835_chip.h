//----------------------------------------------------------------------------------
// Copyright 2022 UniversitĂ¤tsklinikum Freiburg
// Section of Neuroelectronic Systems
// Authors: Bastian Harder
// Project: CIAM
// File Description: This class provides several board functions e.g. SPI, 
// using the C library for bcm2835 from https://www.airspayce.com/mikem/bcm2835/
//----------------------------------------------------------------------------------

#ifndef BCM2835_CHIP_H
#define BCM2835_CHIP_H

#include <stdint.h>
#include <vector>

using namespace std;

#define SUCCESS 1
#define FAIL 0

class BCM2835_Chip
{
public:
    BCM2835_Chip();
	~BCM2835_Chip();
	
	void spi_init(uint32_t spi_speed_hz);
	uint32_t spi_get_clock_freq();
	uint8_t spi_transfer8(uint8_t value);
	void spi_transfer16(char* tbuf, char* rbuf);
	void spi_transfer(vector<uint16_t> &commandList, vector<uint16_t> &receivedList);

private:
	uint32_t _spi_clock_hz;
};

#endif // BCM2835_CHIP_H

