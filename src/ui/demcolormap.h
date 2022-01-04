#ifndef DEMCOLORMAP_H
#define DEMCOLORMAP_H

#include <iostream>
#include <filesystem>
#include <limits>
#include <QMainWindow>
#include "qcustomplot.h"
#include "src/geotiffdem.h"

namespace fs = std::filesystem;

class DEMColorMap : public QCustomPlot, public GeoTiffDEM
{
    Q_OBJECT

public:
    explicit DEMColorMap();
    ~DEMColorMap();

    QCPColorMap *cmap;

    void openDEM(fs::path demPath);
    void plotDEM();
    void fillColorMapData();

public slots:
    void cmapMouseMoveEvent(QMouseEvent *event);

signals:
    void dataCursorChanged(const QString &value);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    std::size_t m_bufXsize = 720, m_bufYsize = 720;
    double m_zbufZmin, m_zbufZmax;
    double m_noDataValue;
    double X0, Y0, X1, Y1;
    GeoTiffDEMAxes m_axes;
    //
    QCPColorGradient m_gradient;
};

#endif // DEMCOLORMAP_H
