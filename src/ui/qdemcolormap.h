#ifndef DEMCOLORMAP_H
#define DEMCOLORMAP_H

#include <iostream>
#include <filesystem>
#include <limits>
#include <QMainWindow>
#include "qcustomplot.h"
#include "src/qgeotiffdem.h"

namespace fs = std::filesystem;

enum QDEMStatusColor
{
    /*! Normal status.*/
    Normal,
    /*! Warning status.*/
    Warning,
    /*! Error status.*/
    Error
};

class QDEMColorMap : public QCustomPlot
{
    Q_OBJECT

public:
    explicit QDEMColorMap();
    ~QDEMColorMap();

    // Getter
    QCPColorMap *cmap();

    void openDEM(const fs::path &demPath);
    void plotDEM(bool axesEquals);
    // Interaction functions
    void resetZoom();
    void zoomIn();
    void zoomOut();


signals:
    void statusChanged(const QString &status, QDEMStatusColor color);
    void cmapCursorPosChanged(const QString &value);
    void progressChanged(const double &progress);    

private slots:
    void onProgressChanged(const double &progress);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    //
    QGeoTiffDEM *m_dem;
    QCPColorMap *m_cmap;
    QCPColorGradient m_gradient;
    std::size_t m_bufXsize, m_bufYsize;
    double m_Xmin, m_Ymin, m_Xmax, m_Ymax,
           m_X0, m_Y0, m_X1, m_Y1,
           m_noDataValue,
           m_zbufZmin, m_zbufZmax,
           m_progress_old, m_progress;
    GeoTiffDEMAxesUnit m_axes;
    GeoTiffDEMinterp m_interp;
    QPoint m_mousePressPos;
    int m_zoomLevel, m_zoomLevelMax, m_zoomInterpThreshold;
    // Private methods
    void replotAxesEquals();
    void getCmapNaturalBoundingBox(double &X0, double &Y1, double &X1, double &Y0);
    void checkDEMPlotBBox();
    //
};

#endif // DEMCOLORMAP_H
