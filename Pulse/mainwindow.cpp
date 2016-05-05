/******************************************************************************************\
|                              C++ Realtime Spectrum Analysis                              |
|                                                                                          |
|                                                                                          |
|                                 Carl Moser and Ian Paul                                  |
|                                                                                          |
|                                                                                          |
|                               Software Design Spring 2016                                |
|                                                                                          |
\******************************************************************************************/

// Header files for qt4
#include "mainwindow.h"
#include "ui_mainwindow.h"

// Header file for the plotting widget
#include "qcustomplot.h"

// Header file for qt's vector for plotting
#include <QVector>

// Header file for the fast fourier transform
#include <fftw3.h>

// Header file for Pulse audio
#include <pulse/simple.h>

// Defining the size of the buffer
#define BUFFER_SIZE 44100/60

/*! \class MainWindow
    \brief The main window for the plot window
    
    This MainWindow class creates the window that the plot is held in and holds the functions for plotting
*/

/*!
  Creates a new MainWindow instance, sets up pulseaudio, and begins the plotting
*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Instantiating the pulse audio client
  	ss.format = PA_SAMPLE_U8;
    ss.channels = 1;
    ss.rate = 44100;
    s = pa_simple_new(NULL,
                    "Peak",
                    PA_STREAM_RECORD,
                    "alsa_output.pci-0000_00_1b.0.analog-stereo.monitor",
                    "Recording",
                    &ss,
                    NULL,
                    NULL,
                    NULL
                    );
    ui->setupUi(this);

    //Begins plotting
    Run(ui->customPlot, 1);
}


/*!
  The destructor for MainWindow
*/
MainWindow::~MainWindow()
{	
    delete ui;
}


/*!
  Runs the plotting of the graph

  int type denotes the type of graph that is plotted
*/
void MainWindow::Run(QCustomPlot *customPlot, int type){
    /*Type denotes the type of graph
      0 - Signal plot
      Anything else - Fourier transform
    */
	  if (type ==0){ //Signal plot
    		customPlot->addGraph();
    		customPlot->graph(0)->setPen(QPen(Qt::blue));
    		customPlot->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);
    		customPlot->xAxis->setLabel("time");
  	  	customPlot->yAxis->setLabel("level");
  	  	customPlot->xAxis->setRange(0, BUFFER_SIZE);
  	  	customPlot->yAxis->setRange(0, 80);


    		connect(&refreshRate, SIGNAL(timeout()), this, SLOT(signalData()));
    		refreshRate.start(0);
    } else{   //Fourier Transform
    		fourier = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    		customPlot->addPlottable(fourier);
    		QPen pen;
    		pen.setWidthF(2);
    	
    		pen.setColor(QColor(1, 92, 191));
    		fourier->setPen(pen);
    		customPlot->yAxis->setRange(0, 1000);
    		customPlot->xAxis->setRange(0, BUFFER_SIZE/2);

    		connect(&refreshRate, SIGNAL(timeout()), this, SLOT(fourierData()));
    		refreshRate.start(0);
  	}
}


/*!
  This function is a private slot that updates the graph

  It plots the audio signal that gets sent to the speakers
*/
void MainWindow::signalData(){

    uint8_t buf[BUFFER_SIZE];
    pa_simple_read(s, buf, sizeof(buf), NULL);
    sigQvectory.clear();
    sigQvectorx.clear();

    for (int i=0; i<sizeof(buf);i++){
      	sigQvectory.append(abs(buf[i] - 128));
      	sigQvectorx.append(i);
    }
    ui->customPlot->graph(0)->clearData();
    ui->customPlot->graph(0)->setData(sigQvectorx, sigQvectory);
    ui->customPlot->replot();

    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 5){
        ui->statusBar->showMessage(QString("%1 FPS").arg(frameCount/(key-lastFpsKey), 0, 'f', 0), 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

/*!
  This function is a private slot that updates the graph

  It plots the FFT of the audio signal
*/
void MainWindow::fourierData(){
  	uint8_t buf[BUFFER_SIZE];
  	N = sizeof(buf);
    in = (double *) fftw_malloc(sizeof(double)*N);
    pa_simple_read(s, buf, sizeof(buf), NULL);

    for (int i = 0; i < N; i++){
        in[i] = abs(buf[i] - 128);
    }

    nc = (N/2)+1;
    out = (double (*)[2])fftw_malloc(sizeof(fftw_complex)*nc);
    plan_forward = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    fftw_execute(plan_forward);

    fourQvectory.clear();
    fourQvectorx.clear();

    for (int i = 0; i<nc; i++){
    	  fourQvectory.append(abs(out[i][0]));
      	fourQvectorx.append(i);
    }

    fourier->setData(fourQvectorx, fourQvectory);
    ui->customPlot->replot();

  	double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
  	static double lastFpsKey;
  	static int frameCount;
  	++frameCount;
    if (key-lastFpsKey > 5){
        ui->statusBar->showMessage(QString("%1 FPS").arg(frameCount/(key-lastFpsKey), 0, 'f', 0), 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}
