#include "geotiffdem.h"

// ============== CONSTRUCTOR ==============
GeoTiffDEM::GeoTiffDEM() {}

// ============== DESTRUCTOR ==============
//! Default destructor
GeoTiffDEM::~GeoTiffDEM()
{
    this->close();
//    OGRSpatialReference::DestroySpatialReference( m_geoSpatialRef ); => Fait crasher le programme !
}

// ============== CLASS METHODS ==============
    // getter
fs::path GeoTiffDEM::getPath() const { return m_demPath; }
std::size_t GeoTiffDEM::getRasterXSize() const { return m_rasterXSize; }
std::size_t GeoTiffDEM::getRasterYSize() const { return m_rasterYSize; }
double GeoTiffDEM::getXmin() const { return m_Xmin; }
double GeoTiffDEM::getXmax() const { return m_Xmax; }
double GeoTiffDEM::getdX() const { return m_dX; }
double GeoTiffDEM::getYmin() const { return m_Ymin; }
double GeoTiffDEM::getYmax() const { return m_Ymax; }
double GeoTiffDEM::getdY() const { return m_dY; }
double GeoTiffDEM::getNoDataValue() const { return m_noDataValue; }
bool GeoTiffDEM::isOpened() const { return m_datasetOpened; }
GeoTiffDEMAxesUnit GeoTiffDEM::getAxesUnit() const { return m_axes; }
std::string GeoTiffDEM::getDEMinfos() const { return m_demInfos; }
    // setter
void GeoTiffDEM::initDrivers() { GDALAllRegister(); }
void GeoTiffDEM::destroyDrivers() { GDALDestroyDriverManager(); }
void GeoTiffDEM::open(fs::path demPath)
{
    if ( !m_datasetOpened )
        this->initializeGeoTiffDEM(demPath);
    else
        throw std::runtime_error("GeoTiffDEM::open Error: A dataset is already opened! "
                                 "Close it with GeoTiffDEM::close() befor opening another dataset.");
}
void GeoTiffDEM::open(const char *demPath)
{
    if ( !m_datasetOpened )
        this->initializeGeoTiffDEM(demPath);
    else
        throw std::runtime_error("GeoTiffDEM::open Error: A dataset is already opened! "
                                 "Close it with GeoTiffDEM::close() befor opening another dataset.");
}
void GeoTiffDEM::close()
{
    if ( m_datasetOpened )
    {
        GDALClose( m_dataset );
        m_datasetOpened = false;
    }
}

//#####################################################//
//########## FromPixelsBoundingBox functions ##########//
//#####################################################//
double **GeoTiffDEM::readFromPixelsBoundingBox(const std::size_t &pXmin, const std::size_t &pYmin,
                                               const std::size_t &pXmax, const std::size_t &pYmax,
                                               std::size_t &zbufXsize, std::size_t &zbufYsize)
{
    // Input parameters order check
    if ( pXmax < pXmin )
        throw std::invalid_argument("GeoTiffDEM::readFromPixelsBoundingBox Error: nXmax can't be "
                                    "smaller than nXmin.");
    if ( pYmax < pYmin )
        throw std::invalid_argument("GeoTiffDEM::readFromPixelsBoundingBox Error: nYmax can't be "
                                    "smaller than nYmin.");
    // Determining X and Y sizes
    zbufXsize = pXmax - pXmin + 1;
    zbufYsize = pYmax - pYmin + 1;
    // Fill zbuffer as function of raster data type
    double **zbuffer = new double*[zbufYsize];
    if ( m_dataType == GDT_Byte )
        this->readToZbuffer<GByte>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer);
    else if ( m_dataType == GDT_UInt16 )
        this->readToZbuffer<GUInt16>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer);
    else if ( m_dataType == GDT_Int16 )
        this->readToZbuffer<GInt16>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer);
    else if ( m_dataType == GDT_UInt32 )
        this->readToZbuffer<GUInt32>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer);
    else if ( m_dataType == GDT_Int32 )
        this->readToZbuffer<GInt32>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer);
    else if ( m_dataType == GDT_Float32 )
        this->readToZbuffer<float>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer);
    else if ( m_dataType == GDT_Float64 )
        this->readToZbuffer<double>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer);
    return zbuffer;
}

double **GeoTiffDEM::readFromPixelsBoundingBox(const std::size_t &pXmin, const std::size_t &pYmin,
                                               const std::size_t &pXmax, const std::size_t &pYmax,
                                               std::size_t &zbufXsize, std::size_t &zbufYsize,
                                               double &zbufZmin, double &zbufZmax)
{
    // Input parameters order check
    if ( pXmax < pXmin )
        throw std::invalid_argument("GeoTiffDEM::readFromPixelsBoundingBox Error: nXmax can't be "
                                    "smaller than nXmin.");
    if ( pYmax < pYmin )
        throw std::invalid_argument("GeoTiffDEM::readFromPixelsBoundingBox Error: nYmax can't be "
                                    "smaller than nYmin.");
    // Determining X and Y sizes
    zbufXsize = pXmax - pXmin + 1;
    zbufYsize = pYmax - pYmin + 1;
    // Fill zbuffer as function of raster data type
    double **zbuffer = new double*[zbufYsize];
    if ( m_dataType == GDT_Byte )
        this->readToZbuffer<GByte>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_UInt16 )
        this->readToZbuffer<GUInt16>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Int16 )
        this->readToZbuffer<GInt16>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_UInt32 )
        this->readToZbuffer<GUInt32>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Int32 )
        this->readToZbuffer<GInt32>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Float32 )
        this->readToZbuffer<float>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Float64 )
        this->readToZbuffer<double>(pXmin, pYmin, zbufXsize, zbufYsize, zbuffer, zbufZmin, zbufZmax);
    return zbuffer;
}

double **GeoTiffDEM::interpFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                                 const double &pX1, const double &pY1,  // Lower-right corner
                                                 const std::size_t &nXsize, const std::size_t &nYsize,
                                                 GeoTiffDEMinterp interp)
{
    if ( interp == Linear )
        return this->interpLinFromPixelsBoundingBox(pX0, pY0, pX1, pY1, nXsize, nYsize);
    else if ( interp == Nearest )
        return this->interpNNFromPixelsBoundingBox(pX0, pY0, pX1, pY1, nXsize, nYsize);
    else
    {
        std::cerr << "GeoTiffDEM::interpFromPixelsBoundingBox: Unrecognized interpolation method" << std::endl;
        return nullptr;
    }
}

double **GeoTiffDEM::interpFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                                 const double &pX1, const double &pY1,  // Lower-right corner
                                                 const std::size_t &nXsize, const std::size_t &nYsize,
                                                 double &zbufZmin, double &zbufZmax,
                                                 GeoTiffDEMinterp interp)
{
    if ( interp == Linear )
        return this->interpLinFromPixelsBoundingBox(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax);
    else if ( interp == Nearest )
        return this->interpNNFromPixelsBoundingBox(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax);
    else
    {
        std::cerr << "GeoTiffDEM::interpFromPixelsBoundingBox: Unrecognized interpolation method" << std::endl;
        return nullptr;
    }
}
//############################################################//
//########## END OF FromPixelsBoundingBox functions ##########//
//############################################################//

//#################################################//
//########## FromXYBoundingBox functions ##########//
//#################################################//
double **GeoTiffDEM::readFromXYboundingBox(const double &Xmin, const double &Ymax,
                                           const double &Xmax, const double &Ymin,
                                           double &XminEx, double &YmaxEx,
                                           double &XmaxEx, double &YminEx,
                                           std::size_t &zbufXsize, std::size_t &zbufYsize)
{
    // Input parameters order check made in 'readFromPixelsboundingBox'
    // Determining corresponding pixel coordinates from natural coordinates
    const std::size_t pXmin = static_cast<std::size_t>((Xmin - m_Xmin) / m_dX),
                      pXmax = static_cast<std::size_t>((Xmax - m_Xmin) / m_dX) + 1,
                      pYmin = static_cast<std::size_t>((Ymax - m_Ymax) / m_dY),
                      pYmax = static_cast<std::size_t>((Ymin - m_Ymax) / m_dY) + 1;
    // Extracted buffer natural bounds
    XminEx = m_Xmin + pXmin * m_dX;
    XmaxEx = m_Xmin + pXmax * m_dX;
    YmaxEx = m_Ymax + pYmin * m_dY;
    YminEx = m_Ymax + pYmax * m_dY;
    //
    return this->readFromPixelsBoundingBox(pXmin, pYmin, pXmax, pYmax, zbufXsize, zbufYsize);
}

double **GeoTiffDEM::readFromXYboundingBox(const double &Xmin, const double &Ymax,
                                           const double &Xmax, const double &Ymin,
                                           double &XminEx, double &YmaxEx,
                                           double &XmaxEx, double &YminEx,
                                           std::size_t &zbufXsize, std::size_t &zbufYsize,
                                           double &zbufZmin, double &zbufZmax)
{
    // Input parameters order check made in 'readFromPixelsboundingBox'
    // Determining corresponding pixel coordinates from natural coordinates
    const std::size_t pXmin = static_cast<std::size_t>((Xmin - m_Xmin) / m_dX),
                      pXmax = static_cast<std::size_t>((Xmax - m_Xmin) / m_dX) + 1,
                      pYmin = static_cast<std::size_t>((Ymax - m_Ymax) / m_dY),
                      pYmax = static_cast<std::size_t>((Ymin - m_Ymax) / m_dY) + 1;
    // Extracted buffer natural bounds
    XminEx = m_Xmin + pXmin * m_dX;
    XmaxEx = m_Xmin + pXmax * m_dX;
    YmaxEx = m_Ymax + pYmin * m_dY;
    YminEx = m_Ymax + pYmax * m_dY;
    //
    return this->readFromPixelsBoundingBox(pXmin, pYmin, pXmax, pYmax,
                                           zbufXsize, zbufYsize, zbufZmin, zbufZmax);
}

double **GeoTiffDEM::interpFromXYboundingBox(const double &Xmin, const double &Ymax,  // Upper-left corner
                                             const double &Xmax, const double &Ymin,  // Lower-right corner
                                             const std::size_t &nXsize, const std::size_t &nYsize,
                                             GeoTiffDEMinterp interp)
{
    const double pX0 = (Xmin - m_Xmin) / m_dX,
                 pX1 = (Xmax - m_Xmin) / m_dX,
                 pY0 = (Ymax - m_Ymax) / m_dY,
                 pY1 = (Ymin - m_Ymax) / m_dY;
    return this->interpFromPixelsBoundingBox(pX0, pY0, pX1, pY1, nXsize, nYsize, interp);
}

double **GeoTiffDEM::interpFromXYboundingBox(const double &Xmin, const double &Ymax,  // Upper-left corner
                                             const double &Xmax, const double &Ymin,  // Lower-right corner
                                             const std::size_t &nXsize, const std::size_t &nYsize,
                                             double &zbufZmin, double &zbufZmax,
                                             GeoTiffDEMinterp interp)
{
    const double pX0 = (Xmin - m_Xmin) / m_dX,
                 pX1 = (Xmax - m_Xmin) / m_dX,
                 pY0 = (Ymax - m_Ymax) / m_dY,
                 pY1 = (Ymin - m_Ymax) / m_dY;
    return this->interpFromPixelsBoundingBox(pX0, pY0, pX1, pY1, nXsize, nYsize, zbufZmin, zbufZmax, interp);
}
//########################################################//
//########## END OF FromXYBoundingBox functions ##########//
//########################################################//

double GeoTiffDEM::getZAtPixels(const double &pX, const double &pY)
{
    // nodata value handling
    double epsilon = std::numeric_limits<double>::epsilon() * 10.0;
    std::size_t pXmin = static_cast<std::size_t>(pX),
                pYmin = static_cast<std::size_t>(pY),
                pXmax = pXmin + 1,
                pYmax = pYmin + 1,
                zbufXsize, zbufYsize;
    if ( pXmax == m_rasterXSize ) // Case of right bound
    {
        pXmax = m_rasterXSize - 1;
        pXmin = pXmax - 1;
    }
    if ( pYmax == m_rasterYSize ) // Case of lower bound
    {
        pYmax = m_rasterYSize - 1;
        pYmin = pYmax - 1;
    }
    double **zbuffer = this->readFromPixelsBoundingBox(pXmin, pYmin, pXmax, pYmax,
                                                       zbufXsize, zbufYsize);
    double z00 = static_cast<double>(zbuffer[0][0]),
           z01 = static_cast<double>(zbuffer[0][1]),
           z10 = static_cast<double>(zbuffer[1][0]),
           z11 = static_cast<double>(zbuffer[1][1]),
           z   = m_noDataValue;
    if ( std::abs( z00 - m_noDataValue ) >= epsilon && std::abs( z01 - m_noDataValue ) >= epsilon &&
         std::abs( z10 - m_noDataValue ) >= epsilon && std::abs( z11 - m_noDataValue ) >= epsilon ) // NO nodata case
    {
        double dX0 = pXmax - pX,
               dY0 = pYmax - pY,
               dX1 = pX - pXmin,
               dY1 = pY - pYmin;
        z = z00 * dX0 * dY0 + z01 * dX1 * dY0 + z10 * dX0 * dY1 + z11 * dX1 * dY1;
    }
    this->deleteZbuffer(zbufYsize, zbuffer);
    return z;
}

double GeoTiffDEM::getZAtXY(const double &X, const double &Y)
{
    return this->getZAtPixels((X - m_Xmin) / m_dX,  // pXmin
                              (Y - m_Ymax) / m_dY); // pYmin
}

void GeoTiffDEM::deleteZbuffer(const std::size_t &rowSize, double **zbuffer)
{
    for ( std::size_t row = 0 ; row < rowSize ; ++row ) // row loop
        delete [] zbuffer[row];
    delete [] zbuffer;
}


// ============== PRIVATE CLASS METHODS ==============
void GeoTiffDEM::initializeGeoTiffDEM(fs::path demPath)
{
    // Initialize path
    m_demPath = demPath;
    // Opening dataset
    m_dataset = (GDALDataset*) GDALOpen( m_demPath.string().c_str(), GA_ReadOnly );
    if ( m_dataset == nullptr )
        throw std::runtime_error("GeoTiffDEM Error: Unable to open\n '" +
                                 m_demPath.string() + "' GeoTiff file.");
    m_datasetOpened = true;
    // Getting raster extensions
    m_rasterXSize = m_dataset->GetRasterXSize();
    m_rasterYSize = m_dataset->GetRasterYSize();
    // Getting geotransform
    double geotransform[6];
    if ( m_dataset->GetGeoTransform(geotransform) == CE_None )
    {
        m_Xmin = geotransform[0]; // Upper left corner
        m_Ymax = geotransform[3];
        m_dX   = geotransform[1];
        m_dY   = geotransform[5];
        // We compute lower right corner
        m_Xmax = m_Xmin + (m_rasterXSize - 1) * m_dX;
        m_Ymin = m_Ymax + (m_rasterYSize - 1) * m_dY;
    }
    else
    {
        std::cerr << "GeoTiffDEM Warning: Impossible to fetch DEM geotransform. Default values will be used." << std::endl;
        m_Xmin = 0.0; // Upper left corner
        m_Ymax = static_cast<double>(m_rasterYSize - 1);
        m_dX   = 1.0;
        m_dY   = -1.0;
        // We compute lower right corner
        m_Xmax = m_Xmin + (m_rasterXSize - 1) * m_dX;
        m_Ymin = m_Ymax + (m_rasterYSize - 1) * m_dY;
    }
    // Checking GeoTiff raster bands (GeoTiff DEM are supposed to have only one band)
    std::size_t nBands = m_dataset->GetRasterCount();
    if ( nBands > 1 )
        std::cerr << "GeoTiffDEM Warning: detected " << nBands << " raster bands. Only first band will be used." << std::endl;
    // Opening GeoTiff dataset raster band
    m_rasterBand = m_dataset->GetRasterBand( 1 );
    // Getting raster band datatype as GDALDataType enum
    m_dataType = m_rasterBand->GetRasterDataType();
    if ( m_dataType != GDT_Byte && m_dataType != GDT_UInt16 && m_dataType != GDT_Int16 &&
         m_dataType != GDT_UInt32 && m_dataType != GDT_Int32 && m_dataType != GDT_Float32 &&
         m_dataType != GDT_Float64 )
        throw std::runtime_error("GeoTiffDEM Error: The file data type is unknown or not handled (maybe complex ?).");
    // Getting No Data value
    m_noDataValue = m_rasterBand->GetNoDataValue();
    // Getting default block sizes
    int nXBlockSize, nYBlockSize;
    m_rasterBand->GetBlockSize( &nXBlockSize, &nYBlockSize );
    if ( nYBlockSize > 1 && static_cast<std::size_t>(nXBlockSize) != m_rasterXSize )
        throw std::runtime_error("GeoTiffDEM Error: The file block sizes are not consistent.");
    m_lineBufferSize = static_cast<std::size_t>(nXBlockSize * nYBlockSize);
    // Getting Spatial References
    const OGRSpatialReference *spatialRef = m_dataset->GetSpatialRef(); // Note GetSpatialRef() returns a const OGRSpatialReference*;
    if ( spatialRef == nullptr )
    {
        std::cerr << "GeoTiffDEM Warning: Projection definition not available." << std::endl;
        m_axes = Pixels;
    }
    else
    {
        std::string axis = spatialRef->GetAttrValue("AXIS", 0);
        if ( axis == "Latitude" || axis == "Longitude" )
            m_axes = LonLat;
        else if ( axis == "Easting" || axis == "Northing" )
            m_axes = EastNorth;
        else
            m_axes = Pixels;
    }
    this->createGeoTiffDEMInfos(spatialRef);
}

void GeoTiffDEM::createGeoTiffDEMInfos(const OGRSpatialReference *spatialRef)
{
    // Get Axes units
    std::string unit{"px"}, axes{"Units: Pixels / Pixels → Altitude [m]"};
    if ( m_axes == LonLat )
    {
        axes = "Longitude [°] / Latitude [°] → Altitude [m]";
        unit = "°";
    }
    else if ( m_axes == EastNorth )
    {
        axes = "Easting [m] / Northing [m] → Altitude [m]";
        unit = "m";
    }
    // Get DEM file size in human readable format
    std::uintmax_t fileSizeBytes = fs::file_size(m_demPath);
    double fileSizeMetric = static_cast<double>(fileSizeBytes); // Power of two multiples
    int i = 0;
    for ( ; fileSizeMetric >= 1000.0 ; fileSizeMetric /= 1000.0, ++i ) {}
    fileSizeMetric = 0.1 * std::round(fileSizeMetric * 10.0); // Rounding to first decimal
    const char *fileSizeMetricPrefix = (i == 0) ? "B" : &"BKMGTPEZY"[i];
    // Get spatialRef as pretty Wkt
    char *wkt_ptr;
    if ( spatialRef == nullptr )
        wkt_ptr = nullptr;
    else
        OGRErr er = spatialRef->exportToPrettyWkt(&wkt_ptr);
    // Fill m_demInfos string
    m_demInfos =  "GeoTiff DEM Infos:\n";
    m_demInfos += "------------------\n";
    m_demInfos += fmt::format("  * File name:                     {}\n", m_demPath.filename().string());
    m_demInfos += fmt::format("  * File size:                     {:.1f} {}B ({} bytes)\n", fileSizeMetric, fileSizeMetricPrefix[0], fileSizeBytes);
    m_demInfos += "  * Raster infos:\n";
    m_demInfos += fmt::format("    → Number of bands:             {}\n", 1);
    m_demInfos += fmt::format("    → Size (width x height):       ({} x {}) pixels\n", m_rasterXSize, m_rasterYSize);
    m_demInfos += fmt::format("    → Pixels data type:            {}\n", GDALGetDataTypeName(m_dataType));
    m_demInfos += fmt::format("    → Pixels color interpretation: {}\n", GDALGetColorInterpretationName(m_rasterBand->GetColorInterpretation()));
    m_demInfos += "  * Geographic infos:\n";
    m_demInfos += fmt::format("    → Axes:                        {}\n", axes);
    m_demInfos += fmt::format("    → Upper-Left corner:           ({}{}, {}{})\n", m_Xmin, unit, m_Ymax, unit);
    m_demInfos += fmt::format("    → Lower-right corner:          ({}{}, {}{})\n", m_Xmax, unit, m_Ymin, unit);
    m_demInfos += fmt::format("    → Pixels sizes:                ({}{}, {}{})\n", m_dX, unit, m_dY, unit);
    m_demInfos += "  * Coordinate Reference System (CRS):\n";
    m_demInfos += fmt::format("{}", (wkt_ptr == nullptr) ? "No CRS available..." : wkt_ptr);
    CPLFree(wkt_ptr);
}

//################################################//
double **GeoTiffDEM::interpNNFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                                   const double &pX1, const double &pY1,  // Lower-right corner
                                                   const std::size_t &nXsize, const std::size_t &nYsize)
{
    // Input parameters order check
    if ( pX1 < pX0 )
        throw std::invalid_argument("GeoTiffDEM::interpNNFromPixelsboundingBox Error: nXmax can't be "
                                    "smaller than nXmin.");
    if ( pY1 < pY0 )
        throw std::invalid_argument("GeoTiffDEM::interpNNFromPixelsboundingBox Error: nYmax can't be "
                                    "smaller than nYmin.");
    // Fill Buffer2D as function of raster data type
    double **zbuffer = new double*[nYsize];
    if ( m_dataType == GDT_Byte )
        this->interpNNToZbuffer<GByte>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_UInt16 )
        this->interpNNToZbuffer<GUInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_Int16 )
        this->interpNNToZbuffer<GInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_UInt32 )
        this->interpNNToZbuffer<GUInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_Int32 )
        this->interpNNToZbuffer<GInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_Float32 )
        this->interpNNToZbuffer<float>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_Float64 )
        this->interpNNToZbuffer<double>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    return zbuffer;
}

double **GeoTiffDEM::interpNNFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                                   const double &pX1, const double &pY1,  // Lower-right corner
                                                   const std::size_t &nXsize, const std::size_t &nYsize,
                                                   double &zbufZmin, double &zbufZmax)
{
    // Input parameters order check
    if ( pX1 < pX0 )
        throw std::invalid_argument("GeoTiffDEM::interpNNFromPixelsboundingBox Error: nXmax can't be "
                                    "smaller than nXmin.");
    if ( pY1 < pY0 )
        throw std::invalid_argument("GeoTiffDEM::interpNNFromPixelsboundingBox Error: nYmax can't be "
                                    "smaller than nYmin.");
    // Fill Buffer2D as function of raster data type
    double **zbuffer = new double*[nYsize];
    if ( m_dataType == GDT_Byte )
        this->interpNNToZbuffer<GByte>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_UInt16 )
        this->interpNNToZbuffer<GUInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Int16 )
        this->interpNNToZbuffer<GInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_UInt32 )
        this->interpNNToZbuffer<GUInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Int32 )
        this->interpNNToZbuffer<GInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Float32 )
        this->interpNNToZbuffer<float>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Float64 )
        this->interpNNToZbuffer<double>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    return zbuffer;
}

    //
double **GeoTiffDEM::interpLinFromPixelsBoundingBox(const double &pX0, const double &pY0,
                                                    const double &pX1, const double &pY1,
                                                    const std::size_t &nXsize, const std::size_t &nYsize)
{
    // Input parameters order check
    if ( pX1 < pX0 )
        throw std::invalid_argument("GeoTiffDEM::interpLinFromPixelsboundingBox Error: nXmax can't be "
                                    "smaller than nXmin.");
    if ( pY1 < pY0 )
        throw std::invalid_argument("GeoTiffDEM::interpLinFromPixelsboundingBox Error: nYmax can't be "
                                    "smaller than nYmin.");
    // Fill Buffer2D as function of raster data type
    double **zbuffer = new double*[nYsize];
    if ( m_dataType == GDT_Byte )
        this->interpLinToZbuffer<GByte>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_UInt16 )
        this->interpLinToZbuffer<GUInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_Int16 )
        this->interpLinToZbuffer<GInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_UInt32 )
        this->interpLinToZbuffer<GUInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_Int32 )
        this->interpLinToZbuffer<GInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_Float32 )
        this->interpLinToZbuffer<float>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    else if ( m_dataType == GDT_Float64 )
        this->interpLinToZbuffer<double>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer);
    return zbuffer;
}

double **GeoTiffDEM::interpLinFromPixelsBoundingBox(const double &pX0, const double &pY0,
                                                    const double &pX1, const double &pY1,
                                                    const std::size_t &nXsize, const std::size_t &nYsize,
                                                    double &zbufZmin, double &zbufZmax)
{
    // Input parameters order check
    if ( pX1 < pX0 )
        throw std::invalid_argument("GeoTiffDEM::interpLinFromPixelsboundingBox Error: nXmax can't be "
                                    "smaller than nXmin.");
    if ( pY1 < pY0 )
        throw std::invalid_argument("GeoTiffDEM::interpLinFromPixelsboundingBox Error: nYmax can't be "
                                    "smaller than nYmin.");
    // Fill Buffer2D as function of raster data type
    double **zbuffer = new double*[nYsize];
    if ( m_dataType == GDT_Byte )
        this->interpLinToZbuffer<GByte>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_UInt16 )
        this->interpLinToZbuffer<GUInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Int16 )
        this->interpLinToZbuffer<GInt16>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_UInt32 )
        this->interpLinToZbuffer<GUInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Int32 )
        this->interpLinToZbuffer<GInt32>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Float32 )
        this->interpLinToZbuffer<float>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    else if ( m_dataType == GDT_Float64 )
        this->interpLinToZbuffer<double>(pX0, pY0, pX1, pY1, nXsize, nYsize, zbuffer, zbufZmin, zbufZmax);
    return zbuffer;
}
//################################################//

template<typename T>
void GeoTiffDEM::readToZbuffer(const std::size_t &pXmin, const std::size_t &pYmin,
                               const std::size_t &zbufXsize, const std::size_t &zbufYsize,
                               double **zbuffer)
{
    // filling resulting array
        // Temporary line reading buffer
    T *lineBuffer = new T[m_lineBufferSize];
    for ( std::size_t iY = 0 ; iY < zbufYsize ; ++iY ) // row loop
    {
        CPLErr er = m_rasterBand->ReadBlock( 0, pYmin + iY, lineBuffer );
        zbuffer[iY] = new double[zbufXsize];
        for ( std::size_t iX = 0 ; iX < zbufXsize ; ++iX ) // col loop
            zbuffer[iY][iX] = static_cast<double>(lineBuffer[pXmin + iX]);
    }
    // Delete temporary line buffer
    delete [] lineBuffer;
}

template<typename T>
void GeoTiffDEM::readToZbuffer(const std::size_t &pXmin, const std::size_t &pYmin,
                               const std::size_t &zbufXsize, const std::size_t &zbufYsize,
                               double **zbuffer, double &zbufZmin, double &zbufZmax)
{
    // Initialization of altitude min max search
    zbufZmin = std::numeric_limits<double>::max();
    zbufZmax = std::numeric_limits<double>::min();
    double epsilon = std::numeric_limits<double>::epsilon() * 10.0,
           z;
    // filling resulting array
        // Temporary line reading buffer
    T *lineBuffer = new T[m_lineBufferSize];
    for ( std::size_t iY = 0 ; iY < zbufYsize ; ++iY ) // row loop
    {
        CPLErr er = m_rasterBand->ReadBlock( 0, pYmin + iY, lineBuffer );
        zbuffer[iY] = new double[zbufXsize];
        for ( std::size_t iX = 0 ; iX < zbufXsize ; ++iX ) // col loop
        {
            z = static_cast<double>(lineBuffer[pXmin + iX]);
            // Compute Altmin/Altmax
            if ( std::abs(z - m_noDataValue) > epsilon ) // Check for no data values
            {
                if ( z > zbufZmax ) zbufZmax = z;
                if ( z < zbufZmin ) zbufZmin = z;
            }
            zbuffer[iY][iX] = z;
        }
    }
    // Delete temporary line buffer
    delete [] lineBuffer;
}

template<typename T>
void GeoTiffDEM::interpNNToZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                                   const double &pX1, const double &pY1,  // Lower-right corner
                                   const std::size_t &nXsize, const std::size_t &nYsize,
                                   double **zbuffer)
{
    // Temporary line buffers
    T *lineBuffer = new T[m_lineBufferSize];
    //
    double dpX = (nXsize > 1) ? ((pX1 - pX0) / (nXsize - 1)) : 0.0, // X interpolation step (Note: handles case of opening only a line, or a point)
           dpY = (nYsize > 1) ? ((pY1 - pY0) / (nYsize - 1)) : 0.0, // Y interpolation step
           pX, pY;             // floating point interpolation values
    //
    std::size_t pXint;
    int pYint, _pYint = -1; // use of 'int' to allow initialization of _pYmin to -1
    for ( std::size_t iY = 0 ; iY < nYsize ; ++iY ) // row loop
    {
        pY = pY0 + iY * dpY;
        pYint = static_cast<int>(pY);
        if ( pY - pYint > 0.5 ) pYint += 1; // if decimal part greater than half a pixel size, we take next index
        if ( pYint > _pYint )
            CPLErr er = m_rasterBand->ReadBlock( 0, pYint, lineBuffer );
        zbuffer[iY] = new double[nXsize];
        for ( std::size_t iX = 0 ; iX < nXsize ; ++iX ) // col loop
        {
            pX = pX0 + iX * dpX;
            pXint = static_cast<std::size_t>(pX); // if decimal part greater than half a pixel size, we take next index
            if ( pX - pXint > 0.5 ) pXint += 1;
            zbuffer[iY][iX] = static_cast<double>(lineBuffer[pXint]);
        }
        _pYint = pYint;
    }
    // Delete temporary line buffers
    delete [] lineBuffer;
}

template<typename T>
void GeoTiffDEM::interpNNToZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                                   const double &pX1, const double &pY1,  // Lower-right corner
                                   const std::size_t &nXsize, const std::size_t &nYsize,
                                   double **zbuffer, double &zbufZmin, double &zbufZmax)
{
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
    for ( std::size_t iY = 0 ; iY < nYsize ; ++iY ) // row loop
    {
        pY = pY0 + iY * dpY;
        pYint = static_cast<int>(pY);
        if ( pY - pYint > 0.5 ) pYint += 1; // if decimal part greater than half a pixel size, we take next index
        if ( pYint > _pYint )
            CPLErr er = m_rasterBand->ReadBlock( 0, pYint, lineBuffer );
        zbuffer[iY] = new double[nXsize];
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
            zbuffer[iY][iX] = z;
        }
        _pYint = pYint;
    }
    // Delete temporary line buffers
    delete [] lineBuffer;
}

template<typename T>
void GeoTiffDEM::interpLinToZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                                    const double &pX1, const double &pY1,  // Lower-right corner
                                    const std::size_t &nXsize, const std::size_t &nYsize,
                                    double **zbuffer)
{
    // nodata value handling
    double epsilon = std::numeric_limits<double>::epsilon() * 10.0;
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
        zbuffer[iY] = new double[nXsize];
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
                zbuffer[iY][iX] = m_noDataValue;
            else if ( std::abs( z01 - m_noDataValue ) < epsilon )
                zbuffer[iY][iX] = m_noDataValue;
            else if ( std::abs( z10 - m_noDataValue ) < epsilon )
                zbuffer[iY][iX] = m_noDataValue;
            else if ( std::abs( z11 - m_noDataValue ) < epsilon )
                zbuffer[iY][iX] = m_noDataValue;
            else
            {
                dX0 = pXmax - pX;
                dX1 = pX - pXmin;
                zbuffer[iY][iX] = z00 * dX0 * dY0 + z01 * dX1 * dY0 +
                                  z10 * dX0 * dY1 + z11 * dX1 * dY1;
            }
        }
        _pYmin = pYmin;
    }
    // Delete temporary line buffers
    delete [] lineBuffer0;
    delete [] lineBuffer1;
}

template<typename T>
void GeoTiffDEM::interpLinToZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                                    const double &pX1, const double &pY1,  // Lower-right corner
                                    const std::size_t &nXsize, const std::size_t &nYsize,
                                    double **zbuffer, double &zbufZmin, double &zbufZmax)
{
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
        zbuffer[iY] = new double[nXsize];
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
                zbuffer[iY][iX] = m_noDataValue;
            else if ( std::abs( z01 - m_noDataValue ) < epsilon )
                zbuffer[iY][iX] = m_noDataValue;
            else if ( std::abs( z10 - m_noDataValue ) < epsilon )
                zbuffer[iY][iX] = m_noDataValue;
            else if ( std::abs( z11 - m_noDataValue ) < epsilon )
                zbuffer[iY][iX] = m_noDataValue;
            else
            {
                dX0 = pXmax - pX;
                dX1 = pX - pXmin;
                z = z00 * dX0 * dY0 + z01 * dX1 * dY0 +
                    z10 * dX0 * dY1 + z11 * dX1 * dY1;
                if ( z > zbufZmax ) zbufZmax = z;
                if ( z < zbufZmin ) zbufZmin = z;
                zbuffer[iY][iX] = z;
            }
        }
        _pYmin = pYmin;
    }
    // Delete temporary line buffers
    delete [] lineBuffer0;
    delete [] lineBuffer1;
}
