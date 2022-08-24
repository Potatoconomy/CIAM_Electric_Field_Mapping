# RHD2000 Acquisition SW for Rasperry Pi
This SW can be used to interface INTAN RHD2216 electrophysiology amplifier chip (16 channel adc).
It has been tested on a raspberry pi 2 but should also work on other rasperry pis.

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
Whenever you collected enough data, you can abort the application by pressing Ctrl+C.
If you want to delete the measurement data, please type `rm measurement.dat` in the terminal.

## Plotting the data
You can plot the data saved in measurements.dat using GNUPLOT.
