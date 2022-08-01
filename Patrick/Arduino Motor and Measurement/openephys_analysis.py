#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Jul 19 16:06:45 2022

@author: Patrick Campbell

Open Ephys data analysis
"""


import open_ephys.analysis
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy.signal import butter, lfilter



data_path = "/Users/nes/Documents/PlatformIO/Projects/Arduino Motor and Measurement/Measurements/openEPhys_data/practice_2022-07-27_12-16-24_try1"
session = open_ephys.analysis.Session(data_path)
record_0 = session.recordnodes[0].recordings[0]


samples = record_0.continuous[0].samples
timestamps = record_0.continuous[0].timestamps
events = record_0.events


def butter_bandpass(lowcut, highcut, fs, order=5):
    return butter(order, [lowcut, highcut], fs=fs, btype='band')

def butter_bandpass_filter(data, lowcut, highcut, fs, order=5):
    b, a = butter_bandpass(lowcut, highcut, fs, order=order)
    y = lfilter(b, a, data)
    return y


filtered = butter_bandpass_filter(samples[:,4], lowcut=0.1, highcut=40, fs=30000, order=2)



def get_time_range(events, samples, timestamps, channel=4):
    ''' Reduces recording to just channel of interest during time period of motor running'''
    # returns 2D array to be plotted, time and samples
    # throw a check in to see if only 2 timestamps
    
    start_time = events.timestamp[0]
    end_time = events.timestamp[1]
    
    start_index = np.where(timestamps == start_time)[0][0]
    end_index = np.where(timestamps == end_time)[0][0]
    
    samples = samples[start_index:end_index, channel]
    timestamps = timestamps[start_index:end_index]
    data = np.array([samples[:], timestamps])
    
    return data


df = get_time_range(events, samples, timestamps, channel=4)
plt.plot(df[1],df[0])