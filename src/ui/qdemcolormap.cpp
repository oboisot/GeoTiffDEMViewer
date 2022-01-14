#include "qdemcolormap.h"

//
//
//
QDEMColorMap::QDEMColorMap()
    : m_dem(new QGeoTiffDEM()),
      m_gradient(QCPColorGradient::gpGrayscale),
      m_interp(GeoTiffDEMinterp::Nearest),
      m_zoomLevel(0), m_zoomLevelMax(10), m_zoomInterpThreshold(6),
      m_isPlotting(false), m_selectRectEnabled(false)
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
    if ( !m_isPlotting )
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
        emit zoomChanged(m_zoomLevel);
    }
}

void QDEMColorMap::plotDEM(bool axesEquals)
{
//    m_isPlotting = true;
//    setCursor(Qt::WaitCursor); // Ensure to have WaitCursor on QCustomPlot
//    emit plotDEMChanged(true, Qt::WaitCursor);
    QFuture<void> future = QtConcurrent::run([this,axesEquals](){
        m_isPlotting = true;
        setCursor(Qt::WaitCursor); // Ensure to have WaitCursor on QCustomPlot
        emit cursorDEMChanged(Qt::WaitCursor);
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
        emit cursorDEMChanged(Qt::ArrowCursor);
        if ( m_selectRectEnabled )
            setCursor(Qt::CrossCursor);
        else
            setCursor(Qt::ArrowCursor);
        m_isPlotting = false;
    });
//    future.waitForFinished();
}

//#################################//
//##### Interaction functions #####//
//#################################//
void QDEMColorMap::resetZoom()
{
    if ( m_dem->isOpened() && !m_isPlotting )
    {
        m_X0 = m_Xmin;
        m_Y0 = m_Ymin;
        m_X1 = m_Xmax;
        m_Y1 = m_Ymax;
        plotDEM(true);
        m_zoomLevel = 0;
        m_interp = GeoTiffDEMinterp::Nearest;
        emit zoomChanged(m_zoomLevel);
    }
}

void QDEMColorMap::zoomIn()
{
    if ( m_dem->isOpened() && !m_isPlotting )
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
            if ( checkDEMPlotBBox() ) plotDEM(false);
        }
        else // We stop zoom min to m_zoomLevelMax
            m_zoomLevel = m_zoomLevelMax;
        emit zoomChanged(m_zoomLevel);
    }
}

void QDEMColorMap::zoomOut()
{
    if ( m_dem->isOpened() && !m_isPlotting )
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
            if ( checkDEMPlotBBox() ) plotDEM(false);
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
            m_zoomLevel = 0;
        emit zoomChanged(m_zoomLevel);
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

bool QDEMColorMap::checkDEMPlotBBox()
{
    if ( m_X0 < m_Xmin ) m_X0 = m_Xmin;
    else if ( m_X0 >= m_Xmax) return false;
    if ( m_Y0 < m_Ymin ) m_Y0 = m_Ymin;
    else if ( m_Y0 >= m_Ymax) return false;
    if ( m_X1 > m_Xmax ) m_X1 = m_Xmax;
    else if ( m_X1 <= m_Xmin) return false;
    if ( m_Y1 > m_Ymax ) m_Y1 = m_Ymax;
    else if ( m_Y1 <= m_Ymin) return false;
    return true;
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
        replotAxesEquals();
    }
}

void QDEMColorMap::mousePressEvent(QMouseEvent *event)
{
    std::cout << m_isPlotting << std::endl;
    if ( !m_isPlotting )
    {
        if ( m_dem->isOpened() )
        {
            if ( event->button() == Qt::LeftButton )
            {
                setCursor(Qt::ClosedHandCursor);
                m_mousePressPos = event->pos();
            }
        }
        else
            emit statusChanged("You must open a DEM.", QDEMStatusColor::Error);
        QCustomPlot::mousePressEvent(event);
    }
    else
        event->ignore();
}

void QDEMColorMap::mouseReleaseEvent(QMouseEvent *event)
{
    if ( !m_isPlotting )
    {
        setCursor(Qt::ArrowCursor);
        if ( m_dem->isOpened() )
        {
            if ( m_zoomLevel > 0 )
            {
                if ( event->button() == Qt::LeftButton )
                {
                    if ( event->pos() != m_mousePressPos )
                    {
                        // Get cmap natural bouding box
                        getCmapNaturalBoundingBox(m_X0, m_Y1, m_X1, m_Y0);
                        if ( checkDEMPlotBBox() ) plotDEM(false);
                    }
                }
            }
        }
        else
            emit statusChanged("You must open a DEM.", QDEMStatusColor::Error);
        QCustomPlot::mouseReleaseEvent(event);
    }
    else
        event->ignore();
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
    else
        emit statusChanged("You must open a DEM.", QDEMStatusColor::Error);
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
            double dX = 0.375 * (X1 - X0), // 0.375 = 3/8 = 3/(2*4)
                   dY = 0.375 * (Y1 - Y0),
                   f = 0.75; // = 2*0.375 (scale factor to be applied on X/Y translation to keep screen clicked position on the same place)
            if ( m_zoomLevel == m_zoomLevelMax + 1) // Case where previous zoom equals maxZoom - 1
            {
                m_zoomLevel = m_zoomLevelMax;
                dX = 0.25 * (X1 - X0);
                dY = 0.25 * (Y1 - Y0);
                f = 0.5;
            }
            if ( m_zoomLevel <= m_zoomLevelMax )
            {
                if ( m_zoomLevel > m_zoomInterpThreshold - 1 )
                    m_interp = GeoTiffDEMinterp::Linear;
                double tX = f * (xAxis->pixelToCoord(event->pos().x()) - xAxis->range().center()), // X translation: Mouse Xposition - Cmap Xcenter (in natural coordinates)
                       tY = f * (yAxis->pixelToCoord(event->pos().y()) - yAxis->range().center()); // Y translation: Mouse Yposition - Cmap Ycenter (in natural coordinates)
                m_X0 = X0 + dX + tX;
                m_Y1 = Y1 - dY + tY;
                m_X1 = X1 - dX + tX;
                m_Y0 = Y0 + dY + tY;
                xAxis->setRange(m_X0, m_X1);
                yAxis->setRange(m_Y0, m_Y1);
                if ( checkDEMPlotBBox() ) plotDEM(false);
            }
            else // We stop zoom max to m_zoomLevelMax
                m_zoomLevel = m_zoomLevelMax;
            emit zoomChanged(m_zoomLevel);
        }
        if ( event->button() == Qt::RightButton )
        {
            emit zoomChanged(m_zoomLevel);
        }
    }
    else
        emit statusChanged("You must open a DEM.", QDEMStatusColor::Error);
    QCustomPlot::mouseDoubleClickEvent(event);
}

