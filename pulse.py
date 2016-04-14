import sys, os, time
from Queue import Queue
from ctypes import POINTER, c_ubyte, c_void_p, c_ulong, cast
import pickle as p
import numpy as np
import matplotlib.pyplot as plt
import scipy as sp

# From https://github.com/Valodim/python-pulseaudio
from pulseaudio.lib_pulseaudio import *

METER_RATE = 44100/8 # Python can't handle a faster rate
MAX_SAMPLE_VALUE = 127
DISPLAY_SCALE = 2
MAX_SPACES = MAX_SAMPLE_VALUE >> DISPLAY_SCALE


class PeakMonitor(object):
    '''The following functions of this class were not written by us:
        - __init__()
        - __iter__()
        - context_notify_cb()
        - sink_info_cb()
        - stream_read_cb()

        We wrote:
        - get_sink_name()
    '''
    def __init__(self, rate):
        self.get_sink_name()
        print (self.sink_name)
        self.rate = rate

        # Wrap callback methods in appropriate ctypefunc instances so
        # that the Pulseaudio C API can call them
        self._context_notify_cb = pa_context_notify_cb_t(self.context_notify_cb)
        self._sink_info_cb = pa_sink_info_cb_t(self.sink_info_cb)
        self._stream_read_cb = pa_stream_request_cb_t(self.stream_read_cb)

        # stream_read_cb() puts peak samples into this Queue instance
        self.samples = Queue()

        # Create the mainloop thread and set our context_notify_cb
        # method to be called when there's updates relating to the
        # connection to Pulseaudio
        _mainloop = pa_threaded_mainloop_new()
        _mainloop_api = pa_threaded_mainloop_get_api(_mainloop)
        context = pa_context_new(_mainloop_api, 'peak_demo')
        pa_context_set_state_callback(context, self._context_notify_cb, None)
        pa_context_connect(context, None, 0, None)
        pa_threaded_mainloop_start(_mainloop)

    def __iter__(self):
        while True:
            yield self.samples.get()

    def context_notify_cb(self, context, _):
        state = pa_context_get_state(context)

        if state == PA_CONTEXT_READY:
            print "Pulseaudio connection ready..."
            # Connected to Pulseaudio. Now request that sink_info_cb
            # be called with information about the available sinks.
            o = pa_context_get_sink_info_list(context, self._sink_info_cb, None)
            pa_operation_unref(o)

        elif state == PA_CONTEXT_FAILED:
            print "Connection failed"

        elif state == PA_CONTEXT_TERMINATED:
            print "Connection terminated"

    def sink_info_cb(self, context, sink_info_p, _, __):
        if not sink_info_p:
            return

        sink_info = sink_info_p.contents
        print '-'* 60
        print 'index:', sink_info.index
        print 'name:', sink_info.name
        print 'description:', sink_info.description

        if sink_info.name == self.sink_name:
            # Found the sink we want to monitor for peak levels.
            # Tell PA to call stream_read_cb with peak samples.
            print
            print 'setting up peak recording using', sink_info.monitor_source_name
            print
            samplespec = pa_sample_spec()
            samplespec.channels = 1
            samplespec.format = PA_SAMPLE_U8
            samplespec.rate = self.rate

            pa_stream = pa_stream_new(context, "peak detect demo", samplespec, None)
            pa_stream_set_read_callback(pa_stream,
                                        self._stream_read_cb,
                                        sink_info.index)
            pa_stream_connect_record(pa_stream,
                                     sink_info.monitor_source_name,
                                     None,
                                     PA_STREAM_PEAK_DETECT)

    def stream_read_cb(self, stream, length, index_incr):
        data = c_void_p()
        pa_stream_peek(stream, data, c_ulong(length))
        data = cast(data, POINTER(c_ubyte))
        for i in xrange(length):
            # When PA_SAMPLE_U8 is used, samples values range from 128
            # to 255 because the underlying audio data is signed but
            # it doesn't make sense to return signed peaks.
            self.samples.put(data[i] - 128)
        pa_stream_drop(stream)

    def get_sink_name(self):
        '''This function was written by us to get the name of the audio sink, as it can change from computer to computer'''
        unparsed = os.popen("pacmd list-sinks").read()
        intermediate = unparsed.replace('>', '<')
        s = intermediate.split('<')
        for i in s:
            if 'analog-stereo' in i and 'alsa_output' in i:
                self.sink_name = i


class analyze():
    def __init__(self, monitor):
        self._monitor = monitor

    def printOut(self):
        for sample in self._monitor:
            print(sample*'>')

    def check(self, length = 44100):
        self._array = np.array([])
        for sample in self._monitor:
            if len(self._array) == length:
                self.outfile = open('file.dat', 'wb')
                p.dump(self._array,self.outfile)
                self.outfile.close()
                self._array = np.append(self._array, sample)
            else:
                self._array = np.append(self._array, sample)

    def processChunks(self, chunkLength=100):
        self._index = range(0,chunkLength)
        self._array = np.zeros(4000)
        self._toAdd = np.array([])
        for sample in self._monitor:
            if len(self._toAdd) == chunkLength:
                #timePlot(self._array)
                freqPlot(self._array)
                self._array = np.delete(self._array, self._index)
                self._array = np.append(self._array,self._toAdd)
                self._toAdd = np.array([])
                self._toAdd = np.append(self._toAdd, sample)
            else:
                self._toAdd = np.append(self._toAdd, sample)

#assumings 44100 sampling rate
def timePlot(points):
    plt.clf()
    timp=len(points)/44100.
    t = np.linspace(0,timp,len(points))
    plt.plot(t,points)
    plt.draw()

def freqPlot(points):
    plt.clf()
    n = len(points)
    Y = sp.fft(points)/n # fft computing and normalization
    Y = Y[range(n/2)]
    line.set_ydata(abs(Y))
    ax.draw_artist(ax.patch)
    ax.draw_artist(line)
    fig.canvas.blit()

def main_print():
    monitor = PeakMonitor(METER_RATE)
    analyzer = analyze(monitor)
    analyzer.printOut()

def main():

    monitor = PeakMonitor(METER_RATE)
    analyzer = analyze(monitor)
    analyzer.processChunks()

if __name__ == '__main__':
    firstTime = True
    fig, ax = plt.subplots()
    line, = ax.semilogx(np.random.randn(2000))
    line.set_xdata(np.linspace(0,20000,2000))
    plt.show(block=False)
    main()