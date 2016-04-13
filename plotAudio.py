#from pylab import plot, show, title, xlabel, ylabel, subplot, savefig, semilogx
import matplotlib.pyplot as plt
from scipy import fft, arange, ifft
from numpy import sin, linspace, pi
from scipy.io.wavfile import read,write

def plotSpectrum(y,Fs):
    n = len(y) # lungime semnal
    k = arange(n)
    T = n/Fs
    frq = k/T # two sides frequency range
    frq = frq[range(n/2)] # one side frequency range

    Y = fft(y)/n # fft computing and normalization
    Y = Y[range(n/2)]

    plt.semilogx(frq,abs(Y),'r') # plotting the spectrum
    plt.xlabel('Freq (Hz)')
    plt.ylabel('|Y(freq)|')
#    plt.canvas.blit()

Fs = 44100.0;  # sampling rate

rate,data=read('PR.wav')
y=data[:,1]
sample_time=5696064.0/56
for time in range(6):
    time = time/10.0
    ySample=y[(time+1)*sample_time:(time+1.3)*sample_time]
    lungime=len(ySample)
    timp=len(ySample)/44100.
    t=linspace(0,timp,len(ySample))
    plt.show()
    plotSpectrum(ySample,Fs)