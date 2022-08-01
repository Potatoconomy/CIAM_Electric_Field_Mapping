# -*- coding: utf-8 -*-
import serial
import pandas as pd
import numpy as np
from datetime import datetime
import matplotlib.pyplot as plt
import os
import open_ephys.analysis
#import scipy.fftpack
from scipy.signal import savgol_filter
from scipy.signal import butter, lfilter


#%%

''' Functions '''


def get_paths():  
    ''' Paths -- ephys name must be set before starting program'''

    time = str(datetime.now())

    fname = 'data_%s.txt' %(time)
    path = "/Users/nes/Documents/PlatformIO/Projects/Arduino Motor and Measurement/Measurements/" + fname

    fig_fname = 'plot_%s.svg' %(time)
    fig_save_path = "/Users/nes/Documents/PlatformIO/Projects/Arduino Motor and Measurement/Measurements/Plots/" + fig_fname
    
    return (path, fig_save_path, fig_fname,)


def get_ephys_path():
    ''' Read in OpenEphys data '''
    # need to make the Ephys dynamic
    data_path_1 = "/Users/nes/Documents/PlatformIO/Projects/Arduino Motor and Measurement/Measurements/openEPhys_data/"
    
    def newest(path):
        files = os.listdir(path)
        for f in files:
            if f.startswith("."):
                files.remove(f)
        paths = [os.path.join(path, basename) for basename in files]
        return max(paths, key=os.path.getctime)
    
    ephys_path = newest(data_path_1)
    
    return(ephys_path)


def gain_thing(gain_setting):
    '''
      //                                                                ADS1015  ADS1115
      //                                                                -------  -------
      // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
      // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
      // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
      // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
      // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
      // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
    '''
    match gain_setting:
        case 0:
            return 0.1875
        case 1:
            return 0.125
        case 2:
            return 0.0625
        case 4:
            return 0.03125
        case 8:
            return 0.015625
        case 16:
            return 0.0078125


def write_serial(serial_port, baud_rate):
    ''' Write Serial Data into file '''    
    ser = serial.Serial(serial_port, baud_rate, timeout=3)
    print('connection made')
    #necessary to start .cpp program in round trip mode
    # If you go too quick, an empty file gets written and error returned
    counter = 0
    try: 
        done = False
        with open(path, 'wb') as f:
            while not done:
                line = ser.readline()
                print(line)
                if counter==0: 
                    ser.write(b'3')
                    counter+=1
                    continue
                if line == b'':      
                    done = True
                    break
                f.write(line)
    except serial.SerialException:
        print("Please close other serial connections")  #engine still runs
        os.remove(path)


def read_data(path):
    ''' Read in saved data from arduino and amplifiers'''
    df = pd.read_csv(path, skiprows=([0,1]), header=0, names=["Current Units","Voltage Units","Position Units","Time [ms]"])
    df.head()
    ''' Set proper gain '''
    try:
        gain_ = pd.read_csv(path, skiprows=(0), nrows=1, header=0, names=["Gain"])
        print(gain_)
        gain_ = gain_.Gain[0]
        print(gain_)
        gain_convertor = gain_thing(gain_)
        print(gain_convertor)
    except IndexError:
        print("Empty dataframe loaded in. No data collected. Closing and deleting file")
        os.remove(path)
        exit
    ''' Manipulate data forms to real units '''
    try: 
        # milliseconds -> seconds, 100[ohms]V[mV] -> I[mA]
        df["Time [s]"] = df["Time [ms]"] / 1000
        df["Current [mA]"] = df["Current Units"] / 100 * gain_convertor # 100 is the resistor amplification value of circuit
        df["Voltage [mV]"] = df["Voltage Units"] * gain_convertor
        distance_covered = 28 #cm  -- mass der strecke
        unit_metric = distance_covered / max(df["Position Units"])
        df["Position [cm]"] = unit_metric * df["Position Units"]
        velocity = np.diff(df["Position [cm]"]) / np.diff(df["Time [s]"])
        velocity = np.append(velocity, 0)
        #velocity = np.gradient(df["Position [cm]"])
        df["Velocity [cm/s]"] = velocity
        
        # Get dV/dt
        #dV = np.diff(df["Voltage [mV]"]) / np.diff(df["Time [s]"])
        #dV = np.append(dV, 0)        
        dV = np.gradient(df["Voltage [mV]"])
        yhat = savgol_filter(dV, 51, 3) # window size 51, polynomial order 3
        df["dV/dt [mV/s]"] = yhat

        
        
    except ValueError:
        print("OOPS! Something went wrong in the data collection!")
        #os.remove(path)
        exit
    
    return(df)


def analyse_ephys(ephys_path, lowcut=0.1, highcut=40, fs=30000, order=2):
    ''' 
    Analyse EPhys data.
    Return 2 column df [timestamps, values] of the round trip triggered by arduino
    '''
    def get_time_range(events, samples, timestamps, channel=4):
        ''' Reduces recording to just channel of interest during time period of motor running'''
        # returns 2D array to be plotted, time and samples
        # throw a check in to see if only 2 timestamps
        triggers = record_0.events[(record_0.events.channel == 1) & (record_0.events.state == 1)] # select the "on" events on channel 1

        start_time = triggers.timestamp[0]
        end_time = triggers.timestamp[2]
        
        start_index = np.where(timestamps == start_time)[0][0]
        end_index = np.where(timestamps == end_time)[0][0]
        
        samples = samples[start_index:end_index, channel]
        timestamps = timestamps[start_index:end_index]
        data = np.array([samples[:], timestamps])
        return data
    
    def butter_bandpass(lowcut, highcut, fs, order=5):
        return butter(order, [lowcut, highcut], fs=fs, btype='band')

    def butter_bandpass_filter(data, lowcut, highcut, fs, order=5):
        b, a = butter_bandpass(lowcut, highcut, fs, order=order)
        y = lfilter(b, a, data)
        return y


    session = open_ephys.analysis.Session(ephys_path)
    record_0 = session.recordnodes[0].recordings[0]
    samples = record_0.continuous[0].samples
    timestamps = record_0.continuous[0].timestamps
    events = record_0.events 
    print(ephys_path)
    
    if len(events) > 0 and len(events) % 2 == 0:
        df_ephys = get_time_range(events, samples, timestamps, channel=4)
    else: 
        print("no events detected")
    
    filtered = butter_bandpass_filter(df_ephys[0], lowcut=0.1, highcut=40, fs=30000, order=2)
    df_ephys = np.vstack([df_ephys, filtered])
    
    return(df_ephys)

def plot_data(fig_save_path, fig_fname):
    ''' Plot collected data '''
    fig, axs = plt.subplots(5)
    #fig.suptitle('')
    fig.tight_layout()
    plt.figure(fig, figsize=(8,6))
    
    axs[0].plot(df["Time [s]"], df["Velocity [cm/s]"])
    axs[1].plot(df["Time [s]"], df["Voltage [mV]"])
    axs[2].plot(df["Time [s]"], df["dV/dt [mV/s]"])
    axs[3].plot(df_ephys[1], df_ephys[0]) # probably requires data reduction method.
    axs[4].plot(df_ephys[1], df_ephys[2]) # filtered data

    axs[0].set_title("Velocity [cm/s] v Time [s]") #cm/s    
    axs[1].set_title("Voltage [mV] v Time [s]") #mV/s
    axs[2].set_title("dV/dt [mV/s]")
    axs[3].set_title("AC Voltage [uV] vs Index")
    axs[4].set_title("Bandpass Filtered AC Voltage")
    
    plt.savefig(fig_save_path, format='svg', dpi=1200)
    return()

#%%
#terminal : ls /dev/cu.*,, python -m serial.tools.list_ports
path, fig__save_path, fig_fname = get_paths()
write_serial(serial_port = '/dev/cu.usbmodem21101', baud_rate = 9600)  
df = read_data(path)
#%%
ephys_path = get_ephys_path()
df_ephys = analyse_ephys(ephys_path)
#%%
plot_data(fig__save_path, fig_fname)




