# RHD2000 Acquisition SW for Rasperry Pi
This SW can be used to interface INTAN RHD2216 electrophysiology amplifier chip (16 channel adc).
It has been tested on a Raspberry Pi 2 but should also work on other Raspberry Pis.

## Hardware requirements
1. Raspberry Pi (including screen, keyboard and mouse or external workstation and LAN cable when using SSH/VNC)
2. Custom Adapter Board for level shifting (Raspberry Shield)
3. INTAN SPI breakout board
4. INTAN SPI interface cable
5. INTAN RHD2216 Headstage with electrodes

## Usage
1. Copy all files to your raspberry pi
2. Change to the directory, build the software and start it:
   ```
    cd RPi_Ctrl_SW
    make
    sudo ./main CHANNEL
   ```
Insert a number between 0 and 15 for CHANNEL to specify the channel number from which the data shall be saved into a file named measurements.dat.
In addition, the measurement results of all channels are printed in the terminal. Unit is µV.
Make sure that the terminal window is big enough! Otherwise the results are not printed on the same line and you get flooded with new lines...
Whenever you collected enough data, you can abort the application by pressing Ctrl+C.
If you want to delete the measurement data, please type `rm measurement.dat` in the terminal.

## Plotting the data
You can plot the data saved in measurements.dat using GNUPLOT.
1. Install GNUPLOT, if not yet installed:
   ```
   sudo apt-get update
   sudo apt-get install gnuplot
   ```
2. Change to the directory of the SW and start GNUPLOT:
   ```
   cd RPi_Ctrl_SW
   gnuplot
   ```
3.Plot the data by typing `plot 'measurements.dat' u 1:2 with lines`
Instead you could also plot the data in real time while it is collected by starting GNUPLOT in seperate terminal than the software. 
Then in GNUPLOT type in `load 'plot_measurements.p`. The plot should reload continuously.

Example plots of very noisy data can be found in the plots folder. 

## Understanding the SW
The SW is written in C++ and can be compiled with the provided makefile.
The purpose of the different SW parts is described in the following.
### bcm2835-1.71
BCM2835 is the name of the Raspberry Pi's chip. This folder contains a open source C library which provides standard I/O functionality like e.g. SPI communication.
Please see [bcm2835-1.71](https://www.airspayce.com/mikem/bcm2835/) for more information.
### bcm2835_chip
This is a simple wrapper class for the bcm2835 library which provides the SPI functions needed in this project.
### rhd2000registers
This is a class provided by INTAN Technology.
It provides functions to generate vectors (dynamic arrays) to configure the rhd2000 and to acquire the adc results.
The vectors generated by the functions of this class can then be taken and send over spi to the rhd2000. 
### data_logger
This class provides the main logging functionality.
It creates instances of bcm2835_chip and rhd2000registers classes to perform all the work.
Command Vectors are generated and sent over spi to the rhd2000. ADC results are received, converted to real voltage values and printed to the terminal.
The results of one channel (channel can be specified) are saved into a file.
### main
Main function. Creates an instance of class data_logger.
SPI clock freq is also specified here and the specified channel for writing adc results to a file is taken from the command line arguments.
