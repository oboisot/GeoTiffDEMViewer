#include "qdemcolormap.h"

//
//
//
QDEMColorMap::QDEMColorMap()
    : m_dem(new QGeoTiffDEM()),
      m_gradient(QCPColorGradient::gpGrayscale)
{
    //
    m_gradient.setNanHandling(QCPColorGradient::nhTransparent);

    // QCustomPlot
    setInteraction(QCP::iRangeDrag);
    axisRect()->setupFullAxesBox(true);

    // QCPColorMap
    m_cmap = new QCPColorMap(xAxis, yAxis);
    m_cmap->setGradient(m_gradient);
    m_cmap->setInterpolate(false);
    //
    connect(m_dem, SIGNAL(progressChanged(const double&)), this, SLOT(onProgressChanged(const double&)));
}
// Destructor
QDEMColorMap::~QDEMColorMap()
{
    delete m_dem;
}

void QDEMColorMap::openDEM(const fs::path &demPath)
{
    m_dem->close();
    m_dem->open(demPath);
    m_bufXsize = axisRect()->size().width();
    m_bufYsize = axisRect()->size().height();
    m_Xmin = m_X0 = m_dem->getXmin();
    m_Ymin = m_Y0 = m_dem->getYmin();
    m_Xmax = m_X1 = m_dem->getXmax();
    m_Ymax = m_Y1 = m_dem->getYmax();
    m_noDataValue = m_dem->getNoDataValue();
    m_axes = m_dem->getAxesUnit();
    m_interp = GeoTiffDEMinterp::Nearest;
    m_zoomLevel = 0;
    m_zoomLevelMax = 10;
    m_zoomInterpThreshold = 6;
    // give the axes some labels:
    if ( m_axes == GeoTiffDEMAxesUnit::LonLat )
    {
        xAxis->setLabel("Longitude [°]");
        yAxis->setLabel("Latitude [°]");
    }
    else if ( m_axes == GeoTiffDEMAxesUnit::NorthEast )
    {
        xAxis->setLabel("Easting [m]");
        yAxis->setLabel("Northing [m]");
    }
    else
    {
        xAxis->setLabel("Pixels");
        yAxis->setLabel("Pixels");
    }
}

void QDEMColorMap::plotDEM(bool axesEquals)
{
    if ( m_dem->isOpened() )
    {
        setCursor(Qt::WaitCursor);
        m_dem->interpFromXYBBoxToQCPColorMapData(m_X0, m_Y1, m_X1, m_Y0,
                                                 m_bufXsize, m_bufYsize,
                                                 m_zbufZmin, m_zbufZmax,
                                                 m_cmap->data(), m_interp);
        m_cmap->data()->setRange(QCPRange(m_X0, m_X1), QCPRange(m_Y0, m_Y1));
        m_cmap->setDataRange(QCPRange(m_zbufZmin, m_zbufZmax));
        m_cmap->rescaleDataRange(true);
        if ( axesEquals )
            replotAxesEquals();
        else
            replot();
        setCursor(Qt::DragMoveCursor);
    }
    else
        emit statusChanged("You must open a DEM.", QDEMStatusColor::Error);
}

//#################################//
//##### Interaction functions #####//
//#################################//
void QDEMColorMap::resetZoom()
{
    if ( m_dem->isOpened() )
    {
        m_X0 = m_Xmin;
        m_Y0 = m_Ymin;
        m_X1 = m_Xmax;
        m_Y1 = m_Ymax;
        plotDEM(true);
        m_zoomLevel = 0;
        m_interp = GeoTiffDEMinterp::Nearest;
    }
}

void QDEMColorMap::zoomIn()
{
    if ( m_dem->isOpened() )
    {
        m_zoomLevel += 1;
        // Get cmap natural bouding box
        double X0, Y1, X1, Y0;
        getCmapNaturalBoundingBox(X0, Y1, X1, Y0);
        double dX = 0.25 * (X1 - X0),
               dY = 0.25 * (Y1 - Y0);
        if ( m_zoomLevel <= m_zoomLevelMax )
        {
            if ( m_zoomLevel > m_zoomInterpThreshold - 1 )
                m_interp = GeoTiffDEMinterp::Linear;
            m_X0 = X0 + dX;
            m_Y1 = Y1 - dY;
            m_X1 = X1 - dX;
            m_Y0 = Y0 + dY;
            // Important !! => allows to zoom on the QCPColorMap center point
            xAxis->setRange(m_X0, m_X1);
            yAxis->setRange(m_Y0, m_Y1);
            checkDEMPlotBBox();
            plotDEM(false);
        }
        else // We stop zoom min to m_zoomLevelMax
            m_zoomLevel = m_zoomLevelMax;
        std::cout << "m_zoomLevel = " << m_zoomLevel << std::endl;
    }
}

void QDEMColorMap::zoomOut()
{
    if ( m_dem->isOpened() )
    {
        m_zoomLevel -= 1;
        // Get cmap natural bouding box
        double X0, Y1, X1, Y0;
        getCmapNaturalBoundingBox(X0, Y1, X1, Y0);
        double dX = 0.5 * (X1 - X0),
               dY = 0.5 * (Y1 - Y0);
        if ( m_zoomLevel < m_zoomInterpThreshold )
            m_interp = GeoTiffDEMinterp::Nearest;
        if ( m_zoomLevel > 0 )
        {
            m_X0 = X0 - dX;
            m_Y1 = Y1 + dY;
            m_X1 = X1 + dX;
            m_Y0 = Y0 - dY;
            // Important !! => allows to zoom on the QCPColorMap center point
            xAxis->setRange(m_X0, m_X1);
            yAxis->setRange(m_Y0, m_Y1);
            checkDEMPlotBBox();
            plotDEM(false);
        }
        else if ( m_zoomLevel == 0 ) // We plot all DEM directly
        {
            m_X0 = m_Xmin;
            m_Y0 = m_Ymin;
            m_X1 = m_Xmax;
            m_Y1 = m_Ymax;
            // Important !! => allows to zoom on the QCPColorMap center point
            xAxis->setRange(X0 - dX, X1 + dX);
            yAxis->setRange(Y0 - dY, Y1 + dY);
            plotDEM(false);
        }
        else // We stop zoom min to 0
        {
            m_zoomLevel = 0;
            emit statusChanged("Minimum zoom reached.", QDEMStatusColor::Warning);
        }
        std::cout << "m_zoomLevel = " << m_zoomLevel << std::endl;
    }
}

//#############################//
//##### Private functions #####//
//#############################//
void QDEMColorMap::replotAxesEquals()
{
    rescaleAxes();
    double X0, Y1, X1, Y0, dX, dY;
    getCmapNaturalBoundingBox(X0, Y1, X1, Y0);
    dX = (X1 - X0) / m_bufXsize; // Handles natural bounds and window size.
    dY = (Y1 - Y0) / m_bufYsize;
    if ( dY > dX )
        xAxis->setScaleRatio(this->yAxis, 1.0);
    else
        yAxis->setScaleRatio(this->xAxis, 1.0);
    replot();
}

void QDEMColorMap::getCmapNaturalBoundingBox(double &X0, double &Y1, double &X1, double &Y0) // Upper-left, lower-right corners
{
    X0 = xAxis->range().lower;
    X1 = xAxis->range().upper;
    Y1 = yAxis->range().upper;
    Y0 = yAxis->range().lower;
}

void QDEMColorMap::checkDEMPlotBBox()
{
    if ( m_X0 < m_Xmin ) m_X0 = m_Xmin;
    if ( m_Y0 < m_Ymin ) m_Y0 = m_Ymin;
    if ( m_X1 > m_Xmax ) m_X1 = m_Xmax;
    if ( m_Y1 > m_Ymax ) m_Y1 = m_Ymax;
}

///////////
// SLOTS //
///////////
void QDEMColorMap::onProgressChanged(const double &progress)
{
    emit progressChanged(progress);
}

////////////
// EVENTS //
////////////
void QDEMColorMap::resizeEvent(QResizeEvent *event)
{
    QCustomPlot::resizeEvent(event);
    if ( m_dem->isOpened() )
    {
        // Update of buffer size relative to cmap rect
        QSize size = this->axisRect()->size();
        m_bufXsize = size.width();
        m_bufYsize = size.height();
//        double X0, Y1, X1, Y0;
//        getCmapNaturalBoundingBox(X0, Y1, X1, Y0);
//        if ( ( X0 < m_X0 && X0 >= m_Xmin ) || ( Y1 > m_Y1 && Y1 <= m_Ymax) ||
//             ( X1 > m_X1 && X1 <= m_Xmax ) || ( Y0 < m_Y0 && Y0 >= m_Ymin) )
//        if ( X0 < m_X0 || Y1 > m_Y1 || X1 > m_X1 || Y0 < m_Y0 )
//        {
//            m_X0 = X0;
//            m_Y1 = Y1;
//            m_X1 = X1;
//            m_Y0 = Y0;
//            checkDEMPlotBBox();
//            plotDEM(true);
//        }
//        else
        replotAxesEquals();
    }
}

void QDEMColorMap::mousePressEvent(QMouseEvent *event)
{
    if ( m_dem->isOpened() )
    {
        if ( event->button() == Qt::LeftButton )
            m_mousePressPos = event->pos();
    }
    QCustomPlot::mousePressEvent(event);
}

void QDEMColorMap::mouseReleaseEvent(QMouseEvent *event)
{
    if ( m_dem->isOpened() && m_zoomLevel > 0 )
    {
        if ( event->button() == Qt::LeftButton )
        {
            if ( event->pos() != m_mousePressPos )
            {
                // Get cmap natural bouding box
                getCmapNaturalBoundingBox(m_X0, m_Y1, m_X1, m_Y0);
                checkDEMPlotBBox();
                plotDEM(false);
            }
        }
    }
    QCustomPlot::mouseReleaseEvent(event);
}

void QDEMColorMap::mouseMoveEvent(QMouseEvent *event)
{
    if ( m_dem->isOpened() )
    {
        // Get cmap natural bouding box
        double X0, Y1, X1, Y0;
        getCmapNaturalBoundingBox(X0, Y1, X1, Y0);
        // Get mouse position in natural coordinates
        double X, Y;
        m_cmap->pixelsToCoords(event->pos().x(), event->pos().y(), X, Y);
        if ( X >= X0 && X <= X1 && Y <= Y1 && Y >= Y0 ) // cursor is in the cmap rect
        {
            QString value;
            double Z = m_cmap->data()->data(X, Y);
            if ( m_cmap->data()->keyRange().contains(X) && m_cmap->data()->valueRange().contains(Y) ) // cursor is in the data rect
            {
                if ( Z != Z ) // nodata case
                {
                    if ( m_axes == GeoTiffDEMAxesUnit::LonLat )
                        value = QString("lon: %1°, lat: %2°, alt: -").arg(QString::number(X, 'f', 6), QString::number(Y, 'f', 6));
                    else if ( m_axes == GeoTiffDEMAxesUnit::NorthEast )
                        value = QString("X: %1m, Y: %2m, alt: -").arg(QString::number(X, 'f', 3), QString::number(X, 'f', 3));
                    else
                        value = QString("X: %1px, Y: %2px, alt: -").arg(QString::number(X, 'f', 3), QString::number(X, 'f', 3));
                }
                else
                {
                    QString Zstr = QString::number(Z, 'f', 3);
                    if ( m_axes == GeoTiffDEMAxesUnit::LonLat )
                        value = QString("lon: %1°, lat: %2°, alt: %3m").arg(QString::number(X, 'f', 6), QString::number(Y, 'f', 6), Zstr);
                    else if ( m_axes == GeoTiffDEMAxesUnit::NorthEast )
                        value = QString("X: %1m, Y: %2m, alt: %3m").arg(QString::number(X, 'f', 3), QString::number(Y, 'f', 3), Zstr);
                    else
                        value = QString("X: %1px, Y: %2px, alt: %3m").arg(QString::number(X, 'f', 3), QString::number(Y, 'f', 3), Zstr);
                }
            }
            else // in the cmap rect but not in the data rect -> like 'nodata'
            {
                if ( m_axes == GeoTiffDEMAxesUnit::LonLat )
                    value = QString("lon: %1°, lat: %2°, alt: -").arg(QString::number(X, 'f', 6), QString::number(Y, 'f', 6));
                else if ( m_axes == GeoTiffDEMAxesUnit::NorthEast )
                    value = QString("X: %1m, Y: %2m, alt: -").arg(QString::number(X, 'f', 3), QString::number(X, 'f', 3));
                else
                    value = QString("X: %1px, Y: %2px, alt: -").arg(QString::number(X, 'f', 3), QString::number(X, 'f', 3));
            }
            emit cmapCursorPosChanged(value);
        }
    }
    QCustomPlot::mouseMoveEvent(event);
}

void QDEMColorMap::mouseDoubleClickEvent(QMouseEvent *event)
{
    if ( m_dem->isOpened() )
    {
        if ( event->button() == Qt::LeftButton )
        {
            m_zoomLevel += 2;
            // Get cmap natural bouding box
            double X0, Y1, X1, Y0;
            getCmapNaturalBoundingBox(X0, Y1, X1, Y0);
            double dX = 0.375 * (X1 - X0),
                   dY = 0.375 * (Y1 - Y0);
            if ( m_zoomLevel == m_zoomLevelMax + 1) // Case where previous zoom equals maxZoom - 1
            {
                m_zoomLevel = m_zoomLevelMax;
                dX = 0.25 * (X1 - X0);
                dY = 0.25 * (Y1 - Y0);
            }
            if ( m_zoomLevel <= m_zoomLevelMax )
            {
                if ( m_zoomLevel > m_zoomInterpThreshold - 1 )
                    m_interp = GeoTiffDEMinterp::Linear;
                m_X0 = X0 + dX;
                m_Y1 = Y1 - dY;
                m_X1 = X1 - dX;
                m_Y0 = Y0 + dY;
                // Important !! => allows to zoom on the QCPColorMap center point
                xAxis->setRange(m_X0, m_X1);
                yAxis->setRange(m_Y0, m_Y1);
                checkDEMPlotBBox();
                plotDEM(false);
            }
            else // We stop zoom max to m_zoomLevelMax
                m_zoomLevel = m_zoomLevelMax;
        }
        if ( event->button() == Qt::RightButton )
        {

        }
    }
    QCustomPlot::mouseDoubleClickEvent(event);
}

