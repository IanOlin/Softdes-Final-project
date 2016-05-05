# Python Spectrogram

##Description 
This repository contains two implementations of a spectrum analyzer: one in Python (depricated) and one in C++. The python is limited in its capabilty and the C++ in still in developement.

##Authors

Carl Moser, Ian Paul

Python-pulseaudio comes from https://github.com/valodim/python-pulseaudio

PeakMonitor comes from http://freshfoo.com/blog/pulseaudio_monitoring

Pulse

FFTW3

Qt4 with QCustomPlotting
QCustomPlotting comes from http://www.qcustomplot.com/index.php/introduction

##Getting started - C++

sudo apt-get install libpulse-dev libmgl-dev libfftw3-dev qt4-dev-tools

run ./run.sh

run ./main

##Getting started - Python
Clone this directory

```
$ sudo apt-get install python-numpy python-scipy
```

To run the shell script that sets up pulseaudio run following commands:
```
$ cd Softdes-Final-Project
$ chmod +x ~/Softdes-Final-Project/install.sh
$ ./install.sh
```

##Running the program
```
$ python2 pulse.py
```

##License
Standard Github License, which isn't really a license at all
