#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QMainWindow>
#include "qcustomplot.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initCustomPlot();

    QCustomPlot *customPlot = new QCustomPlot;

public slots:
    void print(QMouseEvent *event);

private:
    std::size_t m_num_graph = 0;
    QList<QColor> m_color_default = {
        QColor(31, 119, 180),
        QColor(255, 127, 14),
        QColor(44, 160, 44),
        QColor(214, 39, 40),
        QColor(148, 103, 189)
    };
};

#endif // MAINWINDOW_H
