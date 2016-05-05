#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include <QVector>
#include <stdio.h>
#include <fftw3.h>
#include <pulse/simple.h>
#define BUFFER_SIZE 44100/60

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
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
    start();
}

MainWindow::~MainWindow()
{	
	//function for closing UDP and pulse
    delete ui;
}

void MainWindow::start(){
    Run(ui->customPlot, 1);
}

void MainWindow::Run(QCustomPlot *customPlot, int type){
	if (type ==0){
		customPlot->addGraph();
		customPlot->graph(0)->setPen(QPen(Qt::blue));
		customPlot->graph(0)->setScatterStyle(QCPScatterStyle::ssDisc);
		customPlot->xAxis->setLabel("time");
	  	customPlot->yAxis->setLabel("level");
	  	customPlot->xAxis->setRange(0, BUFFER_SIZE);
	  	customPlot->yAxis->setRange(0, 80);


		connect(&refreshRate, SIGNAL(timeout()), this, SLOT(signalData()));
		refreshRate.start(0);
  	} else{
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
  if (key-lastFpsKey > 5) // average fps over 2 seconds
  {
    ui->statusBar->showMessage(
          QString("%1 FPS")
          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
          , 0);
    lastFpsKey = key;
    frameCount = 0;
  }
}

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
    //ui->customPlot->graph(0)->clearData();
    //ui->customPlot->graph(0)->setData(fourQvectorx, fourQvectory);
    //ui->customPlot->graph(0)->rescaleValueAxis();
    //ui->customPlot->replot();
    fourier->setData(fourQvectorx, fourQvectory);
    ui->customPlot->replot();

	double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
	static double lastFpsKey;
	static int frameCount;
	++frameCount;
	if (key-lastFpsKey > 2) // average fps over 2 seconds
	{
	ui->statusBar->showMessage(
	      QString("%1 FPS")
	      .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
	      , 0);
	lastFpsKey = key;
	frameCount = 0;
	}
}
