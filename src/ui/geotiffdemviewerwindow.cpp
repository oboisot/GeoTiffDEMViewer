#include "geotiffdemviewerwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    //MenuBar
//    this->setMenuBar();

    // Qcustomplot
    this->setCentralWidget(customPlot);
    this->setGeometry(0, 0, 600, 500);
    this->setWindowTitle("GeoTiffDEM");
    this->initCustomPlot();    
}

MainWindow::~MainWindow()
{
    delete customPlot;
}

void MainWindow::print(QMouseEvent *event)
{
    std::cout << event->pos().x() << std::endl;
    std::cout << event->pos().y() << std::endl;

}

void MainWindow::initCustomPlot()
{
    // give the axes some labels:
    this->customPlot->xAxis->setLabel("Longitude [°]");
    this->customPlot->yAxis->setLabel("Latitude [°]");
//    this->customPlot->legend->setVisible(true);
    // set axes ranges, so we see all data:
//    this->customPlot->xAxis->setRange(-1, 1);
//    this->customPlot->yAxis->setRange(0, 1);
    this->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    this->customPlot->axisRect()->setupFullAxesBox(true);
    this->customPlot->yAxis->setScaleRatio(this->customPlot->xAxis, 1.0);
//    this->customPlot->replot();
}
