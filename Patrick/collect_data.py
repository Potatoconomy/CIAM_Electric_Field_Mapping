import sys
import os
from pathlib import Path
from datetime import datetime
import argparse
import pickle
import re
import serial


class myClass:
    """
    Class variables:
        Save Paths:
            self.parent_dir
            self.subdir
            self.Plots
            self.SerialData
            self.OpenEPhys
            self.RunInfo
    """

    def __init__(self):

        """ Current Time """
        self.current_time = datetime.now().strftime('%Y-%m-%d %H-%M-%S')

        """ Run Info """
        self.save_name = None
        self.electrodes = None
        self.amp_gain = None
        self.adc_gain = None
        self.setup = None

        """ Save Locations """
        self.parent_dir = None
        self.subdir = None
        self.Plots = None
        self.SerialData = None
        self.OpenEPhys = None
        self.RunInfo = None

        """ Serial Connection """
        self.serial_port = None
        self.baud_rate = None
        self.Ser = None  # Name of serial object to be created

        """ Init Methods """
        self.parse_args()
        self.create_default_save_dir()
        self.create_sub_directories()
        self.get_run_info()
        self.write_run_info()
        self.create_serial_connection()
        return

    def arg_check(self):
        """
        Checks for proper format and input of sys args
        """
        if self.save_name is not None:
            self.save_name = re.sub('[^A-Za-z0-9 ]+', '', self.save_name)  # Remove unacceptable save characters

        while self.adc_gain not in [0, 1, 2, 4, 8, 16] and self.adc_gain is not None:
            print("You have inserted an unacceptable adc_gain.")
            try:
                self.adc_gain = int(input('Please input Gain of ADC:\n'))
            except ValueError:
                print('This is not an integer value in [0, 1, 2, 4, 8, 16]')

        while self.amp_gain not in [6, 51, 501] and self.amp_gain is not None:
            print("You have inserted an unacceptable amp_gain.")
            try:
                self.amp_gain = int(input('Please input Gain of amplifier:\n'))
            except ValueError:
                print('This is not an integer value in [6, 51, 501]')

        while self.baud_rate not in [4800, 9600, 9600, 19200, 38400, 57600, 115200] and self.baud_rate is not None:
            try:
                int(input("Unacceptable baud rate. Acceptable values:\n 4800, 9600, 9600, 19200, 38400, 57600, 115200"))
            except ValueError:
                print('This is not an integer value in [4800, 9600, 9600, 19200, 38400, 57600, 115200].')

    def parse_args(self):
        """
        Read arguments provided to command prompt
        """
        parser = argparse.ArgumentParser()
        parser.add_argument('--save_name', '-s',
                            help="Set the save name.", type=str, default=None)
        parser.add_argument('--electrodes', '-e',
                            help="Declare the electrode type.", type=str, default=None)
        parser.add_argument('--amp_gain', '-g',
                            help="Declare the amplifier gain (6, 51, 501).", type=int, default=None)
        parser.add_argument('--adc_gain', '-a',
                            help="Declare the adc gain (0, 1, 2, 4, 8, 16).", type=int, default=None)
        parser.add_argument('--setup', '-x',
                            help="Declare setup geometry of experiment.", type=str, default=None)
        parser.add_argument('--baud', '-b',
                            help="Declare baud rate [4800 9600 9600 19200 38400 57600 115200]", type=int, default=None)
        parser.add_argument('--serial_port', '-p',
                            help="Serial Port", type=str, default="/dev/cu.usbmodem21101")

        args = parser.parse_args()
        self.save_name = args.save_name
        self.electrodes = args.electrodes
        self.amp_gain = args.amp_gain
        self.adc_gain = args.adc_gain
        self.setup = args.setup

        self.arg_check()

    def create_default_save_dir(self):
        """
        Create default directory for saved data.
        Default directory is in the parent folder of main file execution
        """
        file = sys.argv[0]
        pathname = os.path.abspath(os.path.dirname(file))
        self.parent_dir = os.path.join(pathname, 'Measurements')
        Path(self.parent_dir).mkdir(parents=True, exist_ok=True)
        print('Using Parent Directory:\n\t %s' % self.parent_dir)
        return

    def create_sub_directories(self):
        """
        Setup child directory structure
        """

        def setup_subdirectories(time, name):
            self.subdir = os.path.join(self.parent_dir, time + '_' + name)
            print('Saving to Child Directory:\n\t %s' % self.subdir)

            self.Plots = os.path.join(self.subdir, 'Plots')
            self.SerialData = os.path.join(self.subdir, 'Serial Data')
            self.OpenEPhys = os.path.join(self.subdir, 'OpenEPhys')
            self.RunInfo = os.path.join(self.subdir, 'Run Info')

            Path(self.Plots).mkdir(parents=True, exist_ok=True)
            Path(self.SerialData).mkdir(parents=True, exist_ok=True)
            Path(self.OpenEPhys).mkdir(parents=True, exist_ok=True)
            Path(self.RunInfo).mkdir(parents=True, exist_ok=True)

        if self.save_name is None:
            self.save_name = input('Please give a run save name:\n')  # Could technically put a proper str check here
            self.arg_check()

        setup_subdirectories(self.current_time, self.save_name)
        return

    def get_run_info(self):
        """
        Get sys argv if not already provided
        """
        if self.amp_gain is None:
            self.amp_gain = input('Please input Gain of Amplifier:\n')
            self.arg_check()
        if self.adc_gain is None:
            self.adc_gain = input('Please input ADC Gain setting:\n')
            self.arg_check()
        if self.setup is None:
            self.setup = input('Please input setup of experiment:\n')
        if self.electrodes is None:
            self.electrodes = input('Please input electrode type:\n')
        if self.serial_port is None:
            self.serial_port = input('Please input the serial port:\n')
            self.arg_check()
        if self.baud_rate is None:
            self.adc_gain = input('Please input baud rate:\n')
            self.arg_check()
        return

    def write_run_info(self):
        """
        Save pickle file of experiment details
        """
        dict_ = {'Amplifier Gain': self.amp_gain,
                 'ADC Gain': self.adc_gain,
                 'Electrode Type': self.electrodes,
                 'Setup': self.setup}
        print(dict_)
        pickle.dump(dict_, open(os.path.join(self.RunInfo, self.current_time + '.pkl'), 'wb'))
        return

    def create_serial_connection(self):
        """
        Create Serial connection (with controller)
        """
        self.Ser = serial.Serial(self.serial_port, self.baud_rate, timeout=3)
        print("Connection Made")
        return

    def begin_program_specific_protocol(self):
        line = self.Ser.readline()
       # if line == ''
        return
    
if __name__ == '__main__':
    runner = myClass()
