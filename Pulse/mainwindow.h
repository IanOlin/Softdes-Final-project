#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QVector>
#include <fftw3.h>
#include "qcustomplot.h"
#include <pulse/simple.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void start();
    void Run(QCustomPlot *customPlot, int type);

private slots:
    void signalData();
    void fourierData();
    
private:
    double *in;
    int N;
    int nc;
    fftw_complex *out;
    fftw_plan plan_backward;
    fftw_plan plan_forward;
	pa_simple *s;
	pa_sample_spec ss;
    Ui::MainWindow *ui;
    QVector<double> sigQvectorx;
    QVector<double> sigQvectory;
    QVector<double> fourQvectorx;
    QVector<double> fourQvectory;
    QTimer refreshRate;
    QCPBars *fourier;
};

#endif // MAINWINDOW_H
