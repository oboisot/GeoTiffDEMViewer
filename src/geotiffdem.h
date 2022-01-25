#ifndef GEOTIFFDEM_H
#define GEOTIFFDEM_H

#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <limits>
#include "fmt/format.h"
#include "gdal_priv.h"
#include "ogr_spatialref.h" // for OGRSpatialReference

namespace fs = std::filesystem;

enum GeoTiffDEMAxesUnit
{
    /*! Longitude/Latitude referential in degrees.*/
    LonLat,
    /*! Northing/Easting referential in meters.*/
    NorthEast,
    /*! Undefined referential, fallback to pixels.*/
    Pixels
};

enum GeoTiffDEMinterp
{
    /*! Nearest Neighbor interpolation.*/
    Nearest,
    /*! Bilinear interpolation.*/
    Linear,
};

class GeoTiffDEM
{
public:
    /*! ********************************************************************
    * \brief GeoTiffDEM
    *********************************************************************/
    GeoTiffDEM();

    // ============== DESTRUCTOR ==============
    //! Default destructor
    ~GeoTiffDEM();

    // ============== CLASS METHODS ==============
        // getter
    fs::path getPath() const;
    std::size_t getRasterXSize() const;
    std::size_t getRasterYSize() const;
    double getXmin() const;
    double getXmax() const;
    double getdX() const;
    double getYmin() const;
    double getYmax() const;
    double getdY() const;
    double getNoDataValue() const;
    bool isOpened() const;
    GeoTiffDEMAxesUnit getAxesUnit() const;
    std::string getDEMinfos() const;
        // setter
    void initDrivers();
    void destroyDrivers();
    void open( fs::path demPath );
    void open( const char *demPath );
    void close();

    //! readFromPixelsboundingBox
    /*! Read part of the Geotiff DEM into a 2D buffer array from pixels coordinates
     *  bounding box.
     *
     * \brief readFromPixelsboundingBox
     * \param [in] pXmin The pixel offset from the top left corner of the region to
     *                   be accessed. This would be zero to start from the left side.
     * \param [in] pYmin The line offset from the top left corner of the region to be accessed.
     *                   This would be zero to start from the top.
     * \param [in] pXmax The pixel offset to the top left corner of the region greater than
     *                   nXmin. This defines the pixel range to be accessed.
     * \param [in] pYmax The line offset to the top left corner of the region grater than
     *                   nYmin. This defines the line range to be accessed.
     * \param [out] bufXSize
     * \param [out] bufYSize
     *
     * \return zbuffer
     */
    double **readFromPixelsBoundingBox(const std::size_t &pXmin, const std::size_t &pYmin, // Upper-left corner
                                       const std::size_t &pXmax, const std::size_t &pYmax, // Lower-right corner
                                       std::size_t &zbufXsize, std::size_t &zbufYsize);

    //! readFromPixelsboundingBox
    /*! Oveloaded function which computes the zbuffer min (zbufZmin) and max (zbufZmax)
     *  values taking into account the nodata values.
     */
    double **readFromPixelsBoundingBox(const std::size_t &pXmin, const std::size_t &pYmin, // Upper-left corner
                                       const std::size_t &pXmax, const std::size_t &pYmax, // Lower-right corner
                                       std::size_t &zbufXsize, std::size_t &zbufYsize,
                                       double &zbufZmin, double &zbufZmax);

    double **interpFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                         const double &pX1, const double &pY1,  // Lower-right corner
                                         const std::size_t &nXsize, const std::size_t &nYsize,
                                         GeoTiffDEMinterp interp=GeoTiffDEMinterp::Linear);

    double **interpFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                         const double &pX1, const double &pY1,  // Lower-right corner
                                         const std::size_t &nXsize, const std::size_t &nYsize,
                                         double &zbufZmin, double &zbufZmax,
                                         GeoTiffDEMinterp interp=GeoTiffDEMinterp::Linear);

    //! readFromXYboundingBox
    /*! Read part of the Geotiff DEM into a 2D buffer array from natural coordinates
     *  bounding box, e.g., longitude/latitude or northing/easting.
     */
    double **readFromXYboundingBox(const double &Xmin, const double &Ymax, // Upper-left corner
                                   const double &Xmax, const double &Ymin, // Lower-right corner
                                   double &XminEx, double &YmaxEx, // Returned Upper-left corner
                                   double &XmaxEx, double &YminEx, // Returned Lower-right corner
                                   std::size_t &zbufXsize, std::size_t &zbufYsize);

    //! readFromXYboundingBox
    /*! Oveloaded function which computes the zbuffer min (zbufZmin) and max (zbufZmax)
     *  values taking into account the nodata values.
     */
    double **readFromXYboundingBox(const double &Xmin, const double &Ymax, // Upper-left corner
                                   const double &Xmax, const double &Ymin, // Lower-right corner
                                   double &XminEx, double &YmaxEx, // Returned Upper-left corner
                                   double &XmaxEx, double &YminEx, // Returned Lower-right corner
                                   std::size_t &zbufXsize, std::size_t &zbufYsize,
                                   double &zbufZmin, double &zbufZmax);

    double **interpFromXYboundingBox(const double &Xmin, const double &Ymax,  // Upper-left corner
                                     const double &Xmax, const double &Ymin,  // Lower-right corner
                                     const std::size_t &nXsize, const std::size_t &nYsize,
                                     GeoTiffDEMinterp interp=GeoTiffDEMinterp::Linear);

    double **interpFromXYboundingBox(const double &Xmin, const double &Ymax,  // Upper-left corner
                                     const double &Xmax, const double &Ymin,  // Lower-right corner
                                     const std::size_t &nXsize, const std::size_t &nYsize,
                                     double &zbufZmin, double &zbufZmax,
                                     GeoTiffDEMinterp interp=GeoTiffDEMinterp::Linear);

    //! getAltAtPixels
    /*! Read or interpolate linearly the altitude value at pixels
     *  location.
     */
    double getZAtPixels(const double &pX, const double &pY);

    //! getAltAtXY
    /*! Read or interpolate linearly the altitude value at XY
     *  coordinates location.
     */
    double getZAtXY(const double &X, const double &Y);

    void deleteZbuffer(const std::size_t &rowSize, double **zbuffer);

protected:
    // ============== PROTECTED CLASS MEMBERS ==============
    fs::path            m_demPath;        // GeoTiff data path
    GDALDataset         *m_dataset;       // GeoTiff GDAL dataset
    GDALRasterBand      *m_rasterBand;    // GeoTiff Raster band
    GeoTiffDEMAxesUnit  m_axes;           // GeoTiff axes unit
    std::string         m_demInfos = "";  // GeoTiff DEM infos
        // Check state of opened dataset
    bool m_datasetOpened = false;
        // Size of the dataset
    std::size_t m_rasterXSize,
                m_rasterYSize;
        // Geotransform of the geotiff
    double m_Xmin, m_Xmax, m_dX,
           m_Ymin, m_Ymax, m_dY;
        // GeoTiff NoData value
    double m_noDataValue;
        // Type of the dataset raster band
    GDALDataType m_dataType;
        // Get default line block sizes
    std::size_t m_lineBufferSize;

private:
    // ============== PRIVATE CLASS METHODS ==============
    void initializeGeoTiffDEM(fs::path demPath);
    void createGeoTiffDEMInfos(const OGRSpatialReference *spatialRef);
    //################################################//
    double **interpNNFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                           const double &pX1, const double &pY1,  // Lower-right corner
                                           const std::size_t &nXsize, const std::size_t &nYsize);

    double **interpNNFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                           const double &pX1, const double &pY1,  // Lower-right corner
                                           const std::size_t &nXsize, const std::size_t &nYsize,
                                           double &zbufZmin, double &zbufZmax);

    //! interpLinFromPixelsboundingBox
    /*! Read and interpolate linearly the DEM into z-buffer from (floating) pixels
     *  bounding box.
     *  Contrary to readFromPixelsboundingBox, the returned z-buffer contains values
     *  linearly interpolated at the pixels bounding box values and inside, in
     *  correspondence with the asked number of points in both dimensions.
     *  If the asked size is smaller than the native DEM spacing, data are decimated
     *  linearly, otherwise they are upsampled linearly.
     */
    double **interpLinFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                            const double &pX1, const double &pY1,  // Lower-right corner
                                            const std::size_t &nXsize, const std::size_t &nYsize);

    //! interpLinFromPixelsboundingBox
    /*! Oveloaded function which computes the zbuffer min (zbufZmin) and max (zbufZmax)
     *  values taking into account the nodata values.
     */
    double **interpLinFromPixelsBoundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                            const double &pX1, const double &pY1,  // Lower-right corner
                                            const std::size_t &nXsize, const std::size_t &nYsize,
                                            double &zbufZmin, double &zbufZmax);
    //################################################//
    double **interpNNFromXYboundingBox(const double &Xmin, const double &Ymax,  // Upper-left corner
                                       const double &Xmax, const double &Ymin,  // Lower-right corner
                                       const std::size_t &nXsize, const std::size_t &nYsize);

    double **interpNNFromXYboundingBox(const double &Xmin, const double &Ymax,  // Upper-left corner
                                       const double &Xmax, const double &Ymin,  // Lower-right corner
                                       const std::size_t &nXsize, const std::size_t &nYsize,
                                       double &zbufZmin, double &zbufZmax);

    //! interpLinFromXYboundingBox
    /*! Read and interpolate linearly the DEM into z-buffer from (floating) pixels
     *  bounding box.
     *
     */
    double **interpLinFromXYboundingBox(const double &Xmin, const double &Ymax,  // Upper-left corner
                                        const double &Xmax, const double &Ymin,  // Lower-right corner
                                        const std::size_t &nXsize, const std::size_t &nYsize);

    double **interpLinFromXYboundingBox(const double &Xmin, const double &Ymax,  // Upper-left corner
                                        const double &Xmax, const double &Ymin,  // Lower-right corner
                                        const std::size_t &nXsize, const std::size_t &nYsize,
                                        double &zbufZmin, double &zbufZmax);
    //################################################//
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void readToZbuffer(const std::size_t &pXmin, const std::size_t &pYmin,
                       const std::size_t &zbufXsize, const std::size_t &zbufYsize,
                       double **zbuffer);
    // T -> type of the temporary line reading buffer, depending on raster band type
    // fillZbuffer: overloaded function which computes zmin and zmax while
    // filling z-buffer taking into account nodata values.
    template<typename T>
    void readToZbuffer(const std::size_t &pXmin, const std::size_t &pYmin,
                       const std::size_t &zbufXsize, const std::size_t &zbufYsize,
                       double **zbuffer, double &zbufZmin, double &zbufZmax);
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void interpNNToZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                           const double &pX1, const double &pY1,  // Lower-right corner
                           const std::size_t &nXsize, const std::size_t &nYsize,
                           double **zbuffer);
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void interpNNToZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                           const double &pX1, const double &pY1,  // Lower-right corner
                           const std::size_t &nXsize, const std::size_t &nYsize,
                           double **zbuffer, double &zbufZmin, double &zbufZmax);
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void interpLinToZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                            const double &pX1, const double &pY1,  // Lower-right corner
                            const std::size_t &nXsize, const std::size_t &nYsize,
                            double **zbuffer);
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void interpLinToZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                            const double &pX1, const double &pY1,  // Lower-right corner
                            const std::size_t &nXsize, const std::size_t &nYsize,
                            double **zbuffer, double &zbufZmin, double &zbufZmax);
};

#endif // GEOTIFFDEM_H
