#include "qdemcolormap.h"

//
//
//
QDEMColorMap::QDEMColorMap()
    : m_dem(new QGeoTiffDEM()),
      m_gradient(QCPColorGradient::gpGrayscale),
      m_interp(GeoTiffDEMinterp::Nearest),
      m_zoomLevel(0), m_zoomLevelMax(10), m_zoomInterpThreshold(8),
      m_zoomFactor(1.0), m_betaIn(0.0), m_betaOut(0.0),
      m_isPlotting(false), m_isMousePressedInCmapBBox(false), m_selectRectEnabled(false)
{
    //
    m_gradient.setNanHandling(QCPColorGradient::nhTransparent);

    // QCustomPlot
    setInteraction(QCP::iRangeDrag);
    setInteraction(QCP::iRangeZoom);
    axisRect()->setupFullAxesBox(true);

    // QCPColorMap
    m_cmap = new QCPColorMap(xAxis, yAxis);
    m_cmap->setInterpolate(false);
        // Signals and slots
    connect(m_dem, SIGNAL(progressChanged(const double&)), this, SLOT(onProgressChanged(const double&)));
    // QCPColorScale
    m_cscale = new QCPColorScale(this);
    this->plotLayout()->addElement(0, 1, m_cscale); // add it to the right of the main axis rect
    m_cscale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
//    m_cscale->setMargins(QMargins(0, 0, 0, 0));
    m_cmap->setColorScale(m_cscale); // associate the color map with the color scale
    m_cmap->setGradient(m_gradient); // ColorMap gradient
        // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
    this->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    m_cscale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

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
        // give the colorscale some label
        m_cscale->axis()->setLabel("Altitude [m]");
        // Reset interp method
        m_interp = GeoTiffDEMinterp::Nearest;
        // Reset zoom level
        m_zoomLevel = 0;
        emit zoomChanged(m_zoomLevel);

        // ZoomFactor
        // Here, at zoomLevelMax, a square of 100x100 pixels would be
        // (approximately in the case of non square data pixels) at
        // data full resolution.
        double Xsize = m_dem->getRasterXSize() / 100.0,
               Ysize = m_dem->getRasterYSize() / 100.0,
               invn = 1.0 / m_zoomLevelMax;
        m_zoomFactor = 0.5 * (std::pow(Xsize, invn) + std::pow(Ysize, invn));
        m_betaIn = 0.5 * (1.0 - 1.0 / m_zoomFactor);
        m_betaOut = 0.5 * (1.0 - m_zoomFactor);
        std::cout << "m_zoomFactor = " << m_zoomFactor << std::endl;
    }
}

void QDEMColorMap::plotDEM(bool axesEquals)
{
    QFuture<void> future = QtConcurrent::run([=](){ // send plot operation to another thread
        m_isPlotting = true;
        setCursor(Qt::WaitCursor); // Ensure to have WaitCursor on QCustomPlot
        emit plotChanged(true);
        m_dem->interpFromXYBBoxToQCPColorMapData(m_X0, m_Y1, m_X1, m_Y0,
                                                 m_bufXsize, m_bufYsize,
                                                 m_zbufZmin, m_zbufZmax,
                                                 m_cmap->data(), m_interp);
        m_cmap->data()->setRange(QCPRange(m_X0, m_X1), QCPRange(m_Y0, m_Y1));
        m_cmap->setDataRange(QCPRange(m_zbufZmin, m_zbufZmax));
        m_cmap->rescaleDataRange(true);
        replotDEM(axesEquals);
        if ( m_selectRectEnabled )
            setCursor(Qt::CrossCursor);
        else
            setCursor(Qt::ArrowCursor);
        emit plotChanged(false);
        m_isPlotting = false;
    });
}

void QDEMColorMap::setBackgroundColor(const QColor &color)
{
    setBackground(QBrush(color));
}

void QDEMColorMap::setAxisRectBackgroundColor(const QColor &color)
{
    axisRect()->setBackground(QBrush(color));
}

void QDEMColorMap::setAxesColor(const QColor &color)
{
    // X axes
    xAxis->setLabelColor(color);
    xAxis->setTickLabelColor(color);
    xAxis->setTickPen(QPen(color));
    xAxis->setSubTickPen(QPen(color));
    xAxis->setBasePen(QPen(color));
    xAxis2->setTickPen(QPen(color));
    xAxis2->setSubTickPen(QPen(color));
    xAxis2->setBasePen(QPen(color));
        // Y axes
    yAxis->setLabelColor(color);
    yAxis->setTickLabelColor(color);
    yAxis->setTickPen(QPen(color));
    yAxis->setSubTickPen(QPen(color));
    yAxis->setBasePen(QPen(color));
    yAxis2->setTickPen(QPen(color));
    yAxis2->setSubTickPen(QPen(color));
    yAxis2->setBasePen(QPen(color));
    // ColorScale color
    m_cscale->axis()->setTickLabelColor(color);
    m_cscale->axis()->setLabelColor(color);
    m_cscale->axis()->setTickPen(QPen(color));
    m_cscale->axis()->setSubTickPen(QPen(color));
    m_cscale->axis()->setBasePen(QPen(color));
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
        m_interp = GeoTiffDEMinterp::Nearest;
        m_zoomLevel = 0;
        emit zoomChanged(m_zoomLevel);
    }
}

void QDEMColorMap::zoomIn(const int &zoomStep, const double &tX, const double &tY)
{
    if ( m_dem->isOpened() && !m_isPlotting )
    {
        m_zoomLevel += zoomStep;
        // Get cmap natural bouding box
        double X0, Y1, X1, Y0;
        getCmapBBox(X0, Y1, X1, Y0);
        double zf, tf, dX, dY;
        if ( m_zoomLevel <= m_zoomLevelMax )
        {
            if ( m_zoomLevel >= m_zoomInterpThreshold )
                m_interp = GeoTiffDEMinterp::Linear;
            computeZoomInFactors(zoomStep, zf, tf);
            dX = zf * (X1 - X0);
            dY = zf * (Y1 - Y0);
            m_X0 = X0 + dX + tf * tX;
            m_Y1 = Y1 - dY + tf * tY;
            m_X1 = X1 - dX + tf * tX;
            m_Y0 = Y0 + dY + tf * tY;
            xAxis->setRange(m_X0, m_X1); // Important !! => allows to zoom on the QCPColorMap center point
            yAxis->setRange(m_Y0, m_Y1);
            if ( needPlotFromBBox() ) plotDEM(false);
            emit zoomChanged(m_zoomLevel);
        }
        else // We stop zoom max to m_zoomLevelMax
        {
            int zstep = m_zoomLevelMax + zoomStep - m_zoomLevel; // Always equals 0 when we are at m_zoomLevelMax
                                                                 // Otherwise, compute the needed zstep (lower than zoomStep)
                                                                 // to reach m_zoomLevelMax
            if ( zstep > 0 )
            {
                m_interp = GeoTiffDEMinterp::Linear;
                computeZoomInFactors(zstep, zf, tf);
                dX = zf * (X1 - X0);
                dY = zf * (Y1 - Y0);
                m_X0 = X0 + dX + tf * tX;
                m_Y1 = Y1 - dY + tf * tY;
                m_X1 = X1 - dX + tf * tX;
                m_Y0 = Y0 + dY + tf * tY;
                xAxis->setRange(m_X0, m_X1); // Important !! => allows to zoom on the QCPColorMap center point
                yAxis->setRange(m_Y0, m_Y1);
                if ( needPlotFromBBox() ) plotDEM(false);
            }
            m_zoomLevel = m_zoomLevelMax;
            emit zoomChanged(m_zoomLevel);
        }
    }
}

void QDEMColorMap::zoomOut(const int &zoomStep, const double &tX, const double &tY)
{
    if ( m_dem->isOpened() && !m_isPlotting )
    {
        m_zoomLevel -= zoomStep;
        // Get cmap natural bouding box
        double X0, Y1, X1, Y0;
        getCmapBBox(X0, Y1, X1, Y0);
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
            xAxis->setRange(m_X0, m_X1); // Important !! => allows to zoom on the QCPColorMap center point
            yAxis->setRange(m_Y0, m_Y1);
            if ( needPlotFromBBox() ) plotDEM(false);
        }
        else if ( m_zoomLevel == 0 ) // We plot all DEM directly
        {
            m_X0 = m_Xmin;
            m_Y0 = m_Ymin;
            m_X1 = m_Xmax;
            m_Y1 = m_Ymax;
            xAxis->setRange(X0 - dX, X1 + dX); // Important !! => allows to zoom on the QCPColorMap center point
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
void QDEMColorMap::replotDEM(bool axesEquals)
{
    if ( axesEquals )
    {
        rescaleAxes();
        double X0, Y1, X1, Y0, dX, dY;
        getCmapBBox(X0, Y1, X1, Y0);
        dX = (X1 - X0) / m_bufXsize; // Handles natural bounds and window size.
        dY = (Y1 - Y0) / m_bufYsize;
        if ( dY > dX )
            xAxis->setScaleRatio(this->yAxis, 1.0);
        else
            yAxis->setScaleRatio(this->xAxis, 1.0);
    }
    replot(QCustomPlot::rpQueuedRefresh);
}

void QDEMColorMap::getCmapBBox(double &X0, double &Y1, double &X1, double &Y0) // Upper-left, lower-right corners
{
    X0 = xAxis->range().lower;
    X1 = xAxis->range().upper;
    Y1 = yAxis->range().upper;
    Y0 = yAxis->range().lower;
}

bool QDEMColorMap::needPlotFromBBox()
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

bool QDEMColorMap::isMouseEventInCMapBBox(QPointF position)
{
    double X0, Y1, X1, Y0;
    getCmapBBox(X0, Y1, X1, Y0);
    double X = xAxis->pixelToCoord(position.x()),
           Y = yAxis->pixelToCoord(position.y());
    if ( X >= X0 && X <= X1 && Y >= Y0 && Y <= Y1 )
        return true;
    else
        return false;
}

bool QDEMColorMap::isMouseEventInCScaleRect(QPoint pos)
{
    int posx = pos.x(), posy = pos.y();
    int left = m_cscale->rect().left(),
        right = m_cscale->rect().right(),
        top = m_cscale->rect().top(),
        bottom = m_cscale->rect().bottom();
//    std::cout << "CScaleRect (left, right, top, bottom): (" << left << ", " << right << ", " << top << ", " << bottom << ")" << std::endl;
    if ( posx >= left && posx <= right && posy >= top && posy <= bottom )
        return true;
    else
        return false;
}

void QDEMColorMap::computeZoomInFactors(const int &zoomStep, double &zf, double &tf)
{
    zf = 0.0; // Zoom range factor
    for (int i = 0 ; i < zoomStep ; ++i)
        zf = 1.0 + zf * (1.0 - 2.0 * m_betaIn);
    zf *= m_betaIn;
    tf = 2.0 * zf; // Translation scale factor
}

void QDEMColorMap::computeZoomOutFactors(const int &zoomStep, double &zf, double &tf)
{
    zf = 0.0; // Zoom range factor
    for (int i = 0 ; i < zoomStep ; ++i)
        zf = 1.0 + zf * (1.0 - 2.0 * m_betaOut);
    zf *= m_betaOut;
    tf = 2.0 * zf; // Translation scale factor
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
    // Update of buffer size relative to cmap rect
    QSize size = this->axisRect()->size();
    m_bufXsize = size.width();
    m_bufYsize = size.height();
    if ( !m_isPlotting )
    {
        if ( m_dem->isOpened() )
        {
            replotDEM(true);
        }
    }
}

void QDEMColorMap::mousePressEvent(QMouseEvent *event)
{
    if ( !m_isPlotting )
    {
        if ( m_dem->isOpened() )
        {
            if ( event->button() == Qt::LeftButton )
            {
                if ( isMouseEventInCMapBBox(event->position()) )
                {
                    setCursor(Qt::ClosedHandCursor);
                    m_isMousePressedInCmapBBox = true;
                    m_mousePressPos = event->pos();
                    QCustomPlot::mousePressEvent(event);
                }
                else if ( isMouseEventInCScaleRect(event->pos()) )
                {
                    setCursor(Qt::ClosedHandCursor);
                    m_isMousePressedInCmapBBox = false;
                    QCustomPlot::mousePressEvent(event);
                }
                else
                    event->ignore();
            }
        }
        else
            emit statusChanged("You must open a DEM.", QDEMStatusColor::Error);
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
                    if ( m_isMousePressedInCmapBBox && event->pos() != m_mousePressPos )
                    {
                        // Get cmap natural bouding box
                        getCmapBBox(m_X0, m_Y1, m_X1, m_Y0);
                        if ( needPlotFromBBox() ) plotDEM(false);
                        m_isMousePressedInCmapBBox = false;
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

void QDEMColorMap::mouseDoubleClickEvent(QMouseEvent *event)
{
    if ( !m_isPlotting )
    {
        if ( m_dem->isOpened() )
        {
            if ( event->button() == Qt::LeftButton )
            {
                if ( isMouseEventInCMapBBox(event->position()) )
                {
                    double tX = xAxis->pixelToCoord(event->position().x()) - xAxis->range().center(),
                           tY = yAxis->pixelToCoord(event->position().y()) - yAxis->range().center();
                    zoomIn(2, tX, tY);
                    QCustomPlot::mouseDoubleClickEvent(event);
                }
                else
                    event->ignore();
            }
            if ( event->button() == Qt::RightButton )
            {
                if ( isMouseEventInCMapBBox(event->position()) )
                {
                    QCustomPlot::mouseDoubleClickEvent(event);
                }
                else
                    event->ignore();
            }
            else
                event->ignore();
        }
        else
            emit statusChanged("You must open a DEM.", QDEMStatusColor::Error);
    }
    else
        event->ignore();
}

void QDEMColorMap::mouseMoveEvent(QMouseEvent *event)
{
    if ( !m_isPlotting )
    {
        if ( m_dem->isOpened() )
        {
            // Get cmap natural bouding box
            double X0, Y1, X1, Y0;
            getCmapBBox(X0, Y1, X1, Y0);
            // Get mouse position in natural coordinates
            double X, Y;
            m_cmap->pixelsToCoords(event->position().x(), event->position().y(), X, Y);
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
    else
        event->ignore();
}

