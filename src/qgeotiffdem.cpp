#include "qgeotiffdem.h"

// ============== CONSTRUCTOR ==============
QGeoTiffDEM::QGeoTiffDEM(QObject *parent) : QObject(parent) {}

// ============== DESTRUCTOR ==============
//! Default destructor
QGeoTiffDEM::~QGeoTiffDEM() {}

// ============== PUBLIC CLASS METHODS ==============
void QGeoTiffDEM::interpFromPixelsBBoxToQCPColorMapData(const double &pX0, const double &pY0,  // Upper-left corner
                                                        const double &pX1, const double &pY1,  // Lower-right corner
                                                        const std::size_t &nXsize, const std::size_t &nYsize,
                                                        double &zbufZmin, double &zbufZmax,
                                                        QCPColorMapData *data, GeoTiffDEMinterp interp)
{
    // Input parameters order check
    if ( pX1 < pX0 )
        throw std::invalid_argument("GeoTiffDEM::interpNNFromPixelsboundingBox Error: nXmax can't be "
                                    "smaller than nXmin.");
    if ( pY1 < pY0 )
        throw std::invalid_argument("GeoTiffDEM::interpNNFromPixelsboundingBox Error: nYmax can't be "
                                    "smaller than nYmin.");
    if ( m_dataType == GDT_Byte )
    {
        if ( interp == Linear )
            this->interpLinToQCPColorMapData<GByte>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else if ( interp == Nearest )
            this->interpNNToQCPColorMapData<GByte>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else
            std::cerr << "QGeoTiffDEM::interpFromPixelsBBoxToQCPColorMapData: Unrecognized interpolation method" << std::endl;
    }
    else if ( m_dataType == GDT_UInt16 )
    {
        if ( interp == Linear )
            this->interpLinToQCPColorMapData<GUInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else if ( interp == Nearest )
            this->interpNNToQCPColorMapData<GUInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else
            std::cerr << "QGeoTiffDEM::interpFromPixelsBBoxToQCPColorMapData: Unrecognized interpolation method" << std::endl;
    }
    else if ( m_dataType == GDT_Int16 )
    {
        if ( interp == Linear )
            this->interpLinToQCPColorMapData<GInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else if ( interp == Nearest )
            this->interpNNToQCPColorMapData<GInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else
            std::cerr << "QGeoTiffDEM::interpFromPixelsBBoxToQCPColorMapData: Unrecognized interpolation method" << std::endl;
    }
    else if ( m_dataType == GDT_UInt32 )
    {
        if ( interp == Linear )
            this->interpLinToQCPColorMapData<GUInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else if ( interp == Nearest )
            this->interpNNToQCPColorMapData<GUInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else
            std::cerr << "QGeoTiffDEM::interpFromPixelsBBoxToQCPColorMapData: Unrecognized interpolation method" << std::endl;
    }
    else if ( m_dataType == GDT_Int32 )
    {
        if ( interp == Linear )
            this->interpLinToQCPColorMapData<GInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else if ( interp == Nearest )
            this->interpNNToQCPColorMapData<GInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else
            std::cerr << "QGeoTiffDEM::interpFromPixelsBBoxToQCPColorMapData: Unrecognized interpolation method" << std::endl;
    }
    else if ( m_dataType == GDT_Float32 )
    {
        if ( interp == Linear )
            this->interpLinToQCPColorMapData<float>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else if ( interp == Nearest )
            this->interpNNToQCPColorMapData<float>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else
            std::cerr << "QGeoTiffDEM::interpFromPixelsBBoxToQCPColorMapData: Unrecognized interpolation method" << std::endl;
    }
    else if ( m_dataType == GDT_Float64 )
    {
        if ( interp == Linear )
            this->interpLinToQCPColorMapData<double>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else if ( interp == Nearest )
            this->interpNNToQCPColorMapData<double>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data);
        else
            std::cerr << "QGeoTiffDEM::interpFromPixelsBBoxToQCPColorMapData: Unrecognized interpolation method" << std::endl;
    }
}

void QGeoTiffDEM::interpFromXYBBoxToQCPColorMapData(const double &Xmin, const double &Ymax,  // Upper-left corner
                                                    const double &Xmax, const double &Ymin,  // Lower-right corner
                                                    const std::size_t &nXsize, const std::size_t &nYsize,
                                                    double &zbufZmin, double &zbufZmax,
                                                    QCPColorMapData *data, GeoTiffDEMinterp interp)
{
    const double pX0 = (Xmin - m_Xmin) / m_dX,
                 pX1 = (Xmax - m_Xmin) / m_dX,
                 pY0 = (Ymax - m_Ymax) / m_dY,
                 pY1 = (Ymin - m_Ymax) / m_dY;
    this->interpFromPixelsBBoxToQCPColorMapData(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, data, interp);
}

// ============== PRIVATE CLASS METHODS ==============
// Nearest-Neighbor interpolation on pixels
template<typename T>
void QGeoTiffDEM::interpNNToQCPColorMapData(const double &pX0, const double &pY0,  // Upper-left corner
                                            const double &pX1, const double &pY1,  // Lower-right corner
                                            const std::size_t &nXsize, const std::size_t &nYsize,
                                            double &zbufZmin, double &zbufZmax,
                                            QCPColorMapData *data)
{
    // Init progress
    emit progressChanged(0.0);
    // nodata value handling
    zbufZmin = std::numeric_limits<double>::max();
    zbufZmax = std::numeric_limits<double>::min();
    double epsilon = std::numeric_limits<double>::epsilon() * 10.0,
           z;
    // Temporary line buffers
    T *lineBuffer = new T[m_lineBufferSize];
    //
    double dpX = (nXsize > 1) ? ((pX1 - pX0) / (nXsize - 1)) : 0.0, // X interpolation step (Note: handles case of opening only a line, or a point)
           dpY = (nYsize > 1) ? ((pY1 - pY0) / (nYsize - 1)) : 0.0, // Y interpolation step
           pX, pY;             // floating point interpolation values
    //
    std::size_t pXint;
    int pYint, _pYint = -1; // use of 'int' to allow initialization of _pYmin to -1
    // QCPColorMapData initialization
    data->setSize(nXsize, nYsize);
    for ( std::size_t iY = 0 ; iY < nYsize ; ++iY ) // row loop
    {
        pY = pY0 + iY * dpY;
        pYint = static_cast<int>(pY);
        if ( pY - pYint > 0.5 ) pYint += 1; // if decimal part greater than half a pixel size, we take next index
        if ( pYint > _pYint )
            CPLErr er = m_rasterBand->ReadBlock( 0, pYint, lineBuffer );
        for ( std::size_t iX = 0 ; iX < nXsize ; ++iX ) // col loop
        {
            pX = pX0 + iX * dpX;
            pXint = static_cast<std::size_t>(pX);
            if ( pX - pXint > 0.5 ) pXint += 1; // if decimal part greater than half a pixel size, we take next index
            z = static_cast<double>(lineBuffer[pXint]);
            // Compute Altmin/Altmax
            if ( std::abs(z - m_noDataValue) > epsilon ) // Check for no data values
            {
                if ( z > zbufZmax ) zbufZmax = z;
                if ( z < zbufZmin ) zbufZmin = z;
            }
            else // return NaN
                z = std::numeric_limits<double>::quiet_NaN();
            data->setCell(iX, nYsize - 1 - iY, z);
        }
        _pYint = pYint;
        // Update progress
        emit progressChanged((iY + 1.0) / nYsize);
    }
    // Delete temporary line buffers
    delete [] lineBuffer;
}

// Linear interpolation on pixels
template<typename T>
void QGeoTiffDEM::interpLinToQCPColorMapData(const double &pX0, const double &pY0,  // Upper-left corner
                                             const double &pX1, const double &pY1,  // Lower-right corner
                                             const std::size_t &nXsize, const std::size_t &nYsize,
                                             double &zbufZmin, double &zbufZmax,
                                             QCPColorMapData *data)
{
    // Init progress
    emit progressChanged(0.0);
    // nodata value handling
    zbufZmin = std::numeric_limits<double>::max();
    zbufZmax = std::numeric_limits<double>::min();
    double epsilon = std::numeric_limits<double>::epsilon() * 10.0,
           z;
    // Temporary line buffers
    T *lineBuffer0 = new T[m_lineBufferSize];
    T *lineBuffer1 = new T[m_lineBufferSize];
    //
    double dpX = (nXsize > 1) ? ((pX1 - pX0) / (nXsize - 1)) : 0.0, // X interpolation step (Note: handles case of opening only a line, or a point)
           dpY = (nYsize > 1) ? ((pY1 - pY0) / (nYsize - 1)) : 0.0, // Y interpolation step
           pX, pY,             // floating point interpolation values
           dX0, dX1, dY0, dY1, // fractional part of interpolation values
           z00, z01, z10, z11; // buffer interpolation points.
    //
    std::size_t pXmin, pXmax;
    int pYmin, pYmax, _pYmin = -1, // use of 'int' to allow initialization of _pYmin to -1
        rasterYsize = static_cast<int>(m_rasterYSize);
    // QCPColorMapData initialization
    data->setSize(nXsize, nYsize);
    for ( std::size_t iY = 0 ; iY < nYsize ; ++iY ) // row loop
    {
        pY = pY0 + iY * dpY;
        pYmin = static_cast<int>(pY);
        pYmax = pYmin + 1;
        if ( pYmax == rasterYsize ) // Case of lower bound
        {
            pYmax = rasterYsize - 1;
            pYmin = pYmax - 1;
        }
        dY0 = pYmax - pY;
        dY1 = pY - pYmin;
        if ( pYmin > _pYmin )
        {
            CPLErr er0 = m_rasterBand->ReadBlock( 0, pYmin, lineBuffer0 ),
                   er1 = m_rasterBand->ReadBlock( 0, pYmax, lineBuffer1 );
        }
        for ( std::size_t iX = 0 ; iX < nXsize ; ++iX ) // col loop
        {
            pX = pX0 + iX * dpX;
            pXmin = static_cast<std::size_t>(pX0 + iX * dpX);
            pXmax = pXmin + 1;
            if ( pXmax == m_rasterXSize ) // Case of right bound
            {
                pXmax = m_rasterXSize - 1;
                pXmin = pXmax - 1;
            }
            z00 = static_cast<double>(lineBuffer0[pXmin]);
            z01 = static_cast<double>(lineBuffer0[pXmax]);
            z10 = static_cast<double>(lineBuffer1[pXmin]);
            z11 = static_cast<double>(lineBuffer1[pXmax]);
            if ( std::abs( z00 - m_noDataValue ) < epsilon )
                z = std::numeric_limits<double>::quiet_NaN();
            else if ( std::abs( z01 - m_noDataValue ) < epsilon )
                z = std::numeric_limits<double>::quiet_NaN();
            else if ( std::abs( z10 - m_noDataValue ) < epsilon )
                z = std::numeric_limits<double>::quiet_NaN();
            else if ( std::abs( z11 - m_noDataValue ) < epsilon )
                z = std::numeric_limits<double>::quiet_NaN();
            else
            {
                dX0 = pXmax - pX;
                dX1 = pX - pXmin;
                z = z00 * dX0 * dY0 + z01 * dX1 * dY0 +
                    z10 * dX0 * dY1 + z11 * dX1 * dY1;
                if ( z > zbufZmax ) zbufZmax = z;
                if ( z < zbufZmin ) zbufZmin = z;
            }
            data->setCell(iX, nYsize - 1 - iY, z);
        }
        _pYmin = pYmin;
        // Update progress
        emit progressChanged((iY + 1.0) / nYsize);
    }
    // Delete temporary line buffers
    delete [] lineBuffer0;
    delete [] lineBuffer1;
}
