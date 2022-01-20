#include "qdemcolormap.h"

//
//
//
QDEMColorMap::QDEMColorMap()
    : m_dem(new QGeoTiffDEM()),
      m_gradient(QCPColorGradient::gpGrayscale),
      m_interp(GeoTiffDEMinterp::Nearest),
      m_zoomLevel(0), m_zoomLevelMax(10), m_zoomInterpThreshold(8),
      m_zoomFactor(0.0), m_betaIn(0.0), m_betaOut(0.0),
      m_isPlotting(false), m_isMousePressedInCmapBBox(false), m_selectRectEnabled(false)
{
    this->initBufSizeFromScreenSize(); // Initialize m_bufXsize and m_bufYsize from screen(s) size
    //
    m_gradient.setNanHandling(QCPColorGradient::nhTransparent);
    // QCustomPlot
    this->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    this->axisRect()->setupFullAxesBox(true);
    // QCPColorMap
    m_cmap = new QCPColorMap(this->xAxis, this->yAxis);
    m_cmap->setInterpolate(false);
        // Signals and slots
    connect(m_dem, SIGNAL(progressChanged(const double&)), this, SLOT(onProgressChanged(const double&)));
    // QCPColorScale
    m_cscale = new QCPColorScale(this);
    this->plotLayout()->addElement(0, 1, m_cscale); // add it to the right of the main axis rect
    m_cscale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    m_cmap->setColorScale(m_cscale); // associate the color map with the color scale
    m_cmap->setGradient(m_gradient); // ColorMap gradient
        // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
    this->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    m_cscale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

//    //
//    QCPItemPixmap *locationItem = new QCPItemPixmap(this);
//    locationItem->setPixmap(QPixmap(QString(":/qss/dark/icons/svg/application-exit.svg")));
//    locationItem->position()->setAxes(this->xAxis, this->yAxis);
//    location->setScaled(true);
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
        m_Xmin = m_X0 = m_dem->getXmin();
        m_Ymin = m_Y0 = m_dem->getYmin();
        m_Xmax = m_X1 = m_dem->getXmax();
        m_Ymax = m_Y1 = m_dem->getYmax();
        m_noDataValue = m_dem->getNoDataValue();
        m_axesUnit = m_dem->getAxesUnit();
        // give the axes some labels:
        if ( m_axesUnit == GeoTiffDEMAxesUnit::LonLat )
        {
            this->xAxis->setLabel("Longitude [°]");
            this->yAxis->setLabel("Latitude [°]");
        }
        else if ( m_axesUnit == GeoTiffDEMAxesUnit::NorthEast )
        {
            this->xAxis->setLabel("Easting [m]");
            this->yAxis->setLabel("Northing [m]");
        }
        else
        {
            this->xAxis->setLabel("Pixels");
            this->yAxis->setLabel("Pixels");
        }
        // give the colorscale some label
        m_cscale->axis()->setLabel("Altitude [m]");
        // Reset interp method
        m_interp = GeoTiffDEMinterp::Nearest;
        // Reset zoom level
        m_zoomLevel = 0;
        emit this->zoomChanged(m_zoomLevel);
        // ZoomFactor
        this->computeDEMZoomFactors();
    }
}

void QDEMColorMap::plotDEM(bool axesEquals)
{
    QFuture<void> future = QtConcurrent::run([=](){ // send plot operation to another thread
        m_isPlotting = true;
        this->setCursor(Qt::WaitCursor); // Ensure to have WaitCursor on QCustomPlot
        emit this->cursorChanged(Qt::WaitCursor);
        m_dem->interpFromXYBBoxToQCPColorMapData(m_X0, m_Y1, m_X1, m_Y0,
                                                 m_bufXsize, m_bufYsize,
                                                 m_zbufZmin, m_zbufZmax,
                                                 m_cmap->data(), m_interp);
        m_cmap->data()->setRange(QCPRange(m_X0, m_X1), QCPRange(m_Y0, m_Y1));
        m_cmap->setDataRange(QCPRange(m_zbufZmin, m_zbufZmax));
        m_cmap->rescaleDataRange(true);
        this->replotDEM(axesEquals);
        if ( m_selectRectEnabled )
        {
            this->setCursor(Qt::CrossCursor);
            emit this->cursorChanged(Qt::CrossCursor);
        }
        else
        {
            this->setCursor(Qt::ArrowCursor);
            emit this->cursorChanged(Qt::ArrowCursor);
        }
        m_isPlotting = false;
    });
}

bool QDEMColorMap::isDEMOpened(){ return m_dem->isOpened(); };

void QDEMColorMap::getDEMExtent(double &Xmin, double &Ymax, double &Xmax, double &Ymin)
{
    Xmin = m_Xmin;
    Ymax = m_Ymax;
    Xmax = m_Xmax;
    Ymin = m_Ymin;
}
GeoTiffDEMAxesUnit QDEMColorMap::getDEMAxesUnit() { return m_axesUnit; };
void QDEMColorMap::setBackgroundColor(const QColor &color){ setBackground(QBrush(color));}
void QDEMColorMap::setAxisRectBackgroundColor(const QColor &color){ this->axisRect()->setBackground(QBrush(color)); }
void QDEMColorMap::setAxesColor(const QColor &color)
{
    // X axes
    this->xAxis->setLabelColor(color);
    this->xAxis->setTickLabelColor(color);
    this->xAxis->setTickPen(QPen(color));
    this->xAxis->setSubTickPen(QPen(color));
    this->xAxis->setBasePen(QPen(color));
    this->xAxis2->setTickPen(QPen(color));
    this->xAxis2->setSubTickPen(QPen(color));
    this->xAxis2->setBasePen(QPen(color));
    // Y axes
    this->yAxis->setLabelColor(color);
    this->yAxis->setTickLabelColor(color);
    this->yAxis->setTickPen(QPen(color));
    this->yAxis->setSubTickPen(QPen(color));
    this->yAxis->setBasePen(QPen(color));
    this->yAxis2->setTickPen(QPen(color));
    this->yAxis2->setSubTickPen(QPen(color));
    this->yAxis2->setBasePen(QPen(color));
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
QString QDEMColorMap::getZAtXYasStr(const double &X, const double &Y)
{
    if ( X >= m_Xmin && X <= m_Xmax && Y >= m_Ymin && Y <= m_Ymax )
    {
        // nodata value handling
        double epsilon = std::numeric_limits<double>::epsilon() * 10.0,
               Z = m_dem->getZAtXY(X, Y);
        if ( std::abs( Z - m_dem->getNoDataValue() ) < epsilon ) // nodata case
            return QString("n/a");
        else
            return QString("%1m").arg(QString::number(Z, 'f', 3));
    }
    else
        return QString("n/a");
}

void QDEMColorMap::resetZoom()
{
    if ( m_dem->isOpened() && !m_isPlotting )
    {
        m_X0 = m_Xmin;
        m_Y0 = m_Ymin;
        m_X1 = m_Xmax;
        m_Y1 = m_Ymax;
        this->plotDEM(true);
        m_interp = GeoTiffDEMinterp::Nearest;
        m_zoomLevel = 0;
        emit this->zoomChanged(m_zoomLevel);
    }
}

void QDEMColorMap::zoomIn(const int &zoomStep, const double &tX, const double &tY)
{
    if ( m_dem->isOpened() && !m_isPlotting )
    {
        m_zoomLevel += zoomStep;
        // Get cmap natural bouding box
        double X0, Y1, X1, Y0, zf, tf, dX, dY;
        this->getCmapBBox(X0, Y1, X1, Y0);
        if ( m_zoomLevel <= m_zoomLevelMax )
        {
            if ( m_zoomLevel >= m_zoomInterpThreshold )
                m_interp = GeoTiffDEMinterp::Linear;
            this->computeZoomInFactors(zoomStep, zf, tf);
            dX = zf * (X1 - X0);
            dY = zf * (Y1 - Y0);
            m_X0 = X0 + dX + tf * tX;
            m_Y1 = Y1 - dY + tf * tY;
            m_X1 = X1 - dX + tf * tX;
            m_Y0 = Y0 + dY + tf * tY;
            this->xAxis->setRange(m_X0, m_X1); // Important !! => allows to zoom on the QCPColorMap center point
            this->yAxis->setRange(m_Y0, m_Y1);
            if ( needPlotFromBBox() ) this->plotDEM(false);
            emit this->zoomChanged(m_zoomLevel);
        }
        else // We stop zoom max to m_zoomLevelMax
        {
            int zstep = m_zoomLevelMax + zoomStep - m_zoomLevel; // Always equals 0 when we are at m_zoomLevelMax
                                                                 // Otherwise, compute the needed zstep (lower than zoomStep)
                                                                 // to reach m_zoomLevelMax
            if ( zstep > 0 )
            {
                m_interp = GeoTiffDEMinterp::Linear;
                this->computeZoomInFactors(zstep, zf, tf);
                dX = zf * (X1 - X0);
                dY = zf * (Y1 - Y0);
                m_X0 = X0 + dX + tf * tX;
                m_Y1 = Y1 - dY + tf * tY;
                m_X1 = X1 - dX + tf * tX;
                m_Y0 = Y0 + dY + tf * tY;
                this->xAxis->setRange(m_X0, m_X1); // Important !! => allows to zoom on the QCPColorMap center point
                this->yAxis->setRange(m_Y0, m_Y1);
                if ( needPlotFromBBox() ) this->plotDEM(false);
            }
            m_zoomLevel = m_zoomLevelMax;
            emit this->zoomChanged(m_zoomLevel);
        }
    }
}

void QDEMColorMap::zoomOut(const int &zoomStep, const double &tX, const double &tY)
{
    if ( m_dem->isOpened() && !m_isPlotting )
    {
        m_zoomLevel -= zoomStep;
        // Get cmap natural bouding box
        double X0, Y1, X1, Y0, zf, tf, dX, dY;
        this->getCmapBBox(X0, Y1, X1, Y0);
        if ( m_zoomLevel > 0 )
        {
            if ( m_zoomLevel < m_zoomInterpThreshold )
                m_interp = GeoTiffDEMinterp::Nearest;
            this->computeZoomOutFactors(zoomStep, zf, tf);
            dX = zf * (X1 - X0);
            dY = zf * (Y1 - Y0);
            m_X0 = X0 + dX + tf * tX;
            m_Y1 = Y1 - dY + tf * tY;
            m_X1 = X1 - dX + tf * tX;
            m_Y0 = Y0 + dY + tf * tY;
            this->xAxis->setRange(m_X0, m_X1); // Important !! => allows to zoom on the QCPColorMap center point
            this->yAxis->setRange(m_Y0, m_Y1);
            if ( needPlotFromBBox() ) this->plotDEM(false);
            emit this->zoomChanged(m_zoomLevel);
        }
        else // We stop zoom min to 0
        {
            int zstep = m_zoomLevel + zoomStep; // Always equals 0 when we are at min zoom
                                                // Otherwise, compute the needed zstep (lower than zoomStep)
                                                // to reach m_zoomLevelMax
            if ( zstep > 0 ) // We plot all DEM directly
            {
                m_interp = GeoTiffDEMinterp::Nearest;
                this->computeZoomOutFactors(zstep, zf, tf);
                dX = zf * (X1 - X0);
                dY = zf * (Y1 - Y0);
                m_X0 = m_Xmin;
                m_Y0 = m_Ymin;
                m_X1 = m_Xmax;
                m_Y1 = m_Ymax;
                this->xAxis->setRange(X0 + dX + tf * tX, X1 - dX + tf * tX); // Important !! => allows to zoom on the QCPColorMap center point
                this->yAxis->setRange(Y0 + dY + tf * tY, Y1 - dY + tf * tY);
                this->plotDEM(false);
            }
            m_zoomLevel = 0;
            emit this->zoomChanged(m_zoomLevel);
        }
    }
}

//#############################//
//##### Private functions #####//
//#############################//
void QDEMColorMap::initBufSizeFromScreenSize()
{
    QList<QScreen*> screens = QGuiApplication::screens();
    QSize screenSize;
    int screenWidth = std::numeric_limits<int>::min(),
        screenHeight = std::numeric_limits<int>::min();
    for (qsizetype i = 0 ; i < screens.size() ; ++i)
    {
        screenSize = screens[i]->availableSize();
        if ( screenSize.width() > screenWidth )
            screenWidth = screenSize.width();
        if ( screenSize.height() > screenHeight )
            screenHeight = screenSize.height();
    }
    if ( screenWidth <= 0 )
        m_bufXsize = 1920;
    else
        m_bufXsize = static_cast<std::size_t>(screenWidth);
    if ( screenHeight <= 0 )
        m_bufYsize = 1080;
    else
        m_bufYsize = static_cast<std::size_t>(screenHeight);
}

void QDEMColorMap::replotDEM(bool axesEquals)
{
    if ( axesEquals )
    {
        this->rescaleAxes();
        double X0, Y1, X1, Y0, dX, dY;
        this->getCmapBBox(X0, Y1, X1, Y0);
        dX = (X1 - X0) / this->axisRect()->width(); // Handles natural bounds and window size.
        dY = (Y1 - Y0) / this->axisRect()->height();
        if ( dY > dX )
            this->xAxis->setScaleRatio(this->yAxis, 1.0);
        else
            this->yAxis->setScaleRatio(this->xAxis, 1.0);
    }
    replot(QCustomPlot::rpQueuedReplot); // Note: rpQueuedReplot is preferred over rpQueuedRefresh for initialisation of the first plotting.
}

void QDEMColorMap::getCmapBBox(double &X0, double &Y1, double &X1, double &Y0) // Upper-left, lower-right corners
{
    X0 = this->xAxis->range().lower;
    X1 = this->xAxis->range().upper;
    Y1 = this->yAxis->range().upper;
    Y0 = this->yAxis->range().lower;
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
    this->getCmapBBox(X0, Y1, X1, Y0);
    double X = this->xAxis->pixelToCoord(position.x()),
           Y = this->yAxis->pixelToCoord(position.y());
    if ( X >= X0 && X <= X1 && Y >= Y0 && Y <= Y1 )
        return true;
    else
        return false;
}

bool QDEMColorMap::isMouseEventInCScaleRect(QPointF position)
{
    double posx = position.x(),
           posy = position.y(),
           left = static_cast<double>(m_cscale->rect().left()),
           right = static_cast<double>(m_cscale->rect().right()),
           top = static_cast<double>(m_cscale->rect().top()),
           bottom = static_cast<double>(m_cscale->rect().bottom());
    if ( posx >= left && posx <= right && posy >= top && posy <= bottom )
        return true;
    else
        return false;
}

void QDEMColorMap::computeDEMZoomFactors()
{
    // Here, at zoomLevelMax, a square of 100x100 pixels would be
    // (approximately in the case of non square data pixels) at
    // data full resolution.
    double invn = 1.0 / m_zoomLevelMax;
    m_zoomFactor = 0.5 * (std::pow(m_dem->getRasterXSize() / 100.0, invn) +
                          std::pow(m_dem->getRasterYSize() / 100.0, invn));
    m_betaIn = 0.5 * (1.0 - 1.0 / m_zoomFactor);
    m_betaOut = 0.5 * (1.0 - m_zoomFactor);
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
void QDEMColorMap::onProgressChanged(const double &progress){ emit this->progressChanged(progress); }

////////////
// EVENTS //
////////////
void QDEMColorMap::resizeEvent(QResizeEvent *event)
{
    QCustomPlot::resizeEvent(event);
    if ( !m_isPlotting )
    {
        if ( m_dem->isOpened() )
        {
            this->replotDEM(true);
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
                else if ( isMouseEventInCScaleRect(event->position()) )
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
            emit this->statusChanged("You must open a DEM", QDEMStatusColor::Error, 5000);
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
                        this->getCmapBBox(m_X0, m_Y1, m_X1, m_Y0);
                        if ( needPlotFromBBox() ) this->plotDEM(false);
                        m_isMousePressedInCmapBBox = false;
                    }
                }
            }
        }
        else
            emit this->statusChanged("You must open a DEM", QDEMStatusColor::Error, 5000);
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
                    double tX = this->xAxis->pixelToCoord(event->position().x()) - this->xAxis->range().center(),
                           tY = this->yAxis->pixelToCoord(event->position().y()) - this->yAxis->range().center();
                    this->zoomIn(2, tX, tY);
                    QCustomPlot::mouseDoubleClickEvent(event);
                }
                else
                    event->ignore();
            }
            if ( event->button() == Qt::RightButton )
            {
                if ( isMouseEventInCMapBBox(event->position()) )
                {
                    double tX = this->xAxis->pixelToCoord(event->position().x()) - this->xAxis->range().center(),
                           tY = this->yAxis->pixelToCoord(event->position().y()) - this->yAxis->range().center();
                    this->zoomOut(2, tX, tY);
                    QCustomPlot::mouseDoubleClickEvent(event);
                }
                else
                    event->ignore();
            }
            else
                event->ignore();
        }
        else
            emit this->statusChanged("You must open a DEM", QDEMStatusColor::Error, 5000);
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
            if ( isMouseEventInCMapBBox(event->position()) ) // cursor is in the cmap rect
            {
                QString value, Xstr, Ystr, Zstr;
                // Get mouse position in natural coordinates
                double X, Y, Z;
                m_cmap->pixelsToCoords(event->position().x(), event->position().y(), X, Y);
                Z = m_cmap->data()->data(X, Y); // Get Altitude
                Xstr = QString::number(X, 'f', 6);
                Ystr = QString::number(Y, 'f', 6);
                if ( m_cmap->data()->keyRange().contains(X) && m_cmap->data()->valueRange().contains(Y) ) // cursor is in the data rect
                {
                    if ( Z != Z ) // nodata case
                        Zstr = QString("-");
                    else
                        Zstr = QString("%1m").arg(QString::number(Z, 'f', 3));
                }
                else // in the cmap rect but not in the data rect -> like 'nodata'
                    Zstr = QString("-");
                switch ( m_axesUnit )
                {
                case LonLat:
                    value = QString("lon: %1°, lat: %2°, alt: %3").arg(Xstr, Ystr, Zstr);break;
                case NorthEast:
                    value = QString("X: %1m, Y: %2m, alt: %3").arg(Xstr, Ystr, Zstr);break;
                case Pixels:
                    value = QString("X: %1px, Y: %2px, alt: %3").arg(Xstr, Ystr, Zstr);break;
                default:
                    value = QString("X: %1px, Y: %2px, alt: %3").arg(Xstr, Ystr, Zstr);break;
                }
                emit this->cmapCursorPosChanged(value);
            }
        }
        else
            emit this->statusChanged("You must open a DEM", QDEMStatusColor::Error, 5000);
        QCustomPlot::mouseMoveEvent(event);
    }
    else
        event->ignore();
}

void QDEMColorMap::wheelEvent(QWheelEvent *event)
{
    if ( !m_isPlotting )
    {
        if ( m_dem->isOpened() )
        {
            if ( isMouseEventInCScaleRect(event->position()) )
                QCustomPlot::wheelEvent(event);
            else
                event->ignore();
        }
        else
        {
            event->ignore();
            emit this->statusChanged("You must open a DEM", QDEMStatusColor::Error, 5000);
        }
    }
    else
        event->ignore();
}
