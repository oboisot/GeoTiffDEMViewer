#include "demcolormap.h"

//
//
//
DEMColorMap::DEMColorMap()
{
    //
    m_gradient = QCPColorGradient::gpGrayscale;
    m_gradient.setNanHandling(QCPColorGradient::nhTransparent);

    // QCustomPlot
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    this->axisRect()->setupFullAxesBox(true);

    // QCPColorMap
    cmap = new QCPColorMap(this->xAxis, this->yAxis);
    cmap->setGradient(m_gradient);
    cmap->setInterpolate(false);
    //
    connect(this, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(cmapMouseMoveEvent(QMouseEvent*)));
}
// Destructor
DEMColorMap::~DEMColorMap() {}

void DEMColorMap::openDEM(fs::path demPath)
{
    this->open(demPath);
    m_noDataValue = this->getNoDataValue();
    X0 = this->getXmin();
    Y0 = this->getYmin();
    X1 = this->getXmax();
    Y1 = this->getYmax();
    m_axes = this->getAxesUnit();
    // give the axes some labels:
    if ( m_axes == GeoTiffDEMAxes::LonLat )
    {
        this->xAxis->setLabel("Longitude [°]");
        this->yAxis->setLabel("Latitude [°]");
    }
    else if ( m_axes == GeoTiffDEMAxes::NorthEast )
    {
        this->xAxis->setLabel("Easting [m]");
        this->yAxis->setLabel("Northing [m]");
    }
    else
    {
        this->xAxis->setLabel("Pixels");
        this->yAxis->setLabel("Pixels");
    }
}

void DEMColorMap::plotDEM()
{
    if ( this->isOpened() )
    {
        cmap->data()->setSize(m_bufXsize, m_bufYsize);
        cmap->data()->setRange(QCPRange(X0, X1), QCPRange(Y0, Y1));
        this->fillColorMapData();
        cmap->setDataRange(QCPRange(m_zbufZmin, m_zbufZmax));
        cmap->rescaleDataRange();
        // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
        QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
        this->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
        this->rescaleAxes();
        if ( Y1 - Y0 > X1 - X0 )
            this->xAxis->setScaleRatio(this->yAxis, 1.0);
        else
            this->yAxis->setScaleRatio(this->xAxis, 1.0);
    }
    else
        std::cerr << "You must open a DEM first." << std::endl;
}

void DEMColorMap::fillColorMapData()
{
    // Open and fill zbuffer
    double **zbuffer = this->readInterpolateFromXYboundingBox(X0, Y0, X1, Y1,
                                                              m_bufXsize, m_bufYsize,
                                                              m_zbufZmin, m_zbufZmax);
    double epsilon = std::numeric_limits<double>::epsilon() * 10.0, z;
    for ( std::size_t iY = 0 ; iY < m_bufYsize ; ++iY )
    {
        for ( std::size_t iX = 0 ; iX < m_bufXsize ; ++iX )
        {
            z = zbuffer[iY][iX];
            if ( std::abs(z - m_noDataValue) < epsilon ) // Check for no data values
                z = std::numeric_limits<double>::quiet_NaN();
            cmap->data()->setCell(iX, m_bufYsize - 1 - iY, z);
        }
    }
    this->deleteZbuffer(m_bufYsize, zbuffer);
}

void DEMColorMap::cmapMouseMoveEvent(QMouseEvent *event)
{
    double X, Y;
    cmap->pixelsToCoords(event->pos().x(), event->pos().y(), X, Y);
    if ( cmap->data()->keyRange().contains(X) && cmap->data()->valueRange().contains(Y) )
    {
        this->setCursor(Qt::CrossCursor);
        QString value;
        double Z = cmap->data()->data(X, Y);
        if ( Z != Z) // nodata case
        {
            if ( m_axes == GeoTiffDEMAxes::LonLat )
                value = QString("lon: %1°, lat: %2°, alt: -").arg(QString::number(X, 'f', 6), QString::number(Y, 'f', 6));
            else if ( m_axes == GeoTiffDEMAxes::NorthEast )
                value = QString("X: %1m, Y: %2m, alt: -").arg(QString::number(X, 'f', 3), QString::number(X, 'f', 3));
            else
                value = QString("X: %1px, Y: %2px, alt: -").arg(QString::number(X, 'f', 3), QString::number(X, 'f', 3));
        }
        else
        {
            QString Zstr = QString::number(Z, 'f', 3);
            if ( m_axes == GeoTiffDEMAxes::LonLat )
                value = QString("lon: %1°, lat: %2°, alt: %3m").arg(QString::number(X, 'f', 6), QString::number(Y, 'f', 6), Zstr);
            else if ( m_axes == GeoTiffDEMAxes::NorthEast )
                value = QString("X: %1m, Y: %2m, alt: %3m").arg(QString::number(X, 'f', 3), QString::number(Y, 'f', 3), Zstr);
            else
                value = QString("X: %1px, Y: %2px, alt! %3m").arg(QString::number(X, 'f', 3), QString::number(Y, 'f', 3), Zstr);
        }
        emit dataCursorChanged(value);
    }
    else
        this->setCursor(Qt::ArrowCursor);
}

void DEMColorMap::resizeEvent(QResizeEvent *event)
{
    std::cout << "resizeEvent" << std::endl;
    if ( cmap->data()->valueRange().size() > cmap->data()->keyRange().size() )
        this->xAxis->setScaleRatio(this->yAxis, 1.0);
    else
        this->yAxis->setScaleRatio(this->xAxis, 1.0);
    QCustomPlot::resizeEvent(event);
}
