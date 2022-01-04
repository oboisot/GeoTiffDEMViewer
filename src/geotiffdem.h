#ifndef GEOTIFFDEM_H
#define GEOTIFFDEM_H

#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <limits>
#include "gdal_priv.h"
#include "ogr_spatialref.h" // for OGRSpatialReference

enum GeoTiffDEMAxes
{
    /*! Longitude/Latitude referential in degrees.*/
    LonLat,
    /*! Northing/Easting referential in meters.*/
    NorthEast,
    /*! Undefined referential, fallback to pixels.*/
    Pixels
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
    std::filesystem::path getPath() const;
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
    GeoTiffDEMAxes getAxesUnit() const;
    void printPrettySpatialRef() const;
        // setter
    void open( std::filesystem::path demPath );
    void open( const char *demPath );
    void close();

    double **readFromPixelsboundingBox(const std::size_t &pXmin, const std::size_t &pYmin,
                                       const std::size_t &pXmax, const std::size_t &pYmax,
                                       std::size_t &zbufXsize, std::size_t &zbufYsize);

    //! readFromPixelsboundingBox
    /*! Oveloaded function which computes the zbuffer min (zbufZmin) and max (zbufZmax)
     *  values taking into account the nodata values.
     */
    double **readFromPixelsboundingBox(const std::size_t &pXmin, const std::size_t &pYmin,
                                       const std::size_t &pXmax, const std::size_t &pYmax,
                                       std::size_t &zbufXsize, std::size_t &zbufYsize,
                                       double &zbufZmin, double &zbufZmax);
    //! readFromPixelsboundingBox
    /*! Read part of the Geotiff DEM into a 2D buffer array from pixels coordinates
     *  bounding box.
     *
     * \brief readFromPixelsboundingBox
     * \param [in] pXmin The pixel offset to the top left corner of the region to
     *                   be accessed. This would be zero to start from the left side.
     * \param [in] pYmin The line offset to the top left corner of the region to be accessed.
     *                   This would be zero to start from the top.
     * \param [in] pXmax The pixel offset to the top left corner of the region greater than
     *                   nXmin. This defines the pixel range to be accessed.
     * \param [in] pYmax The line offset to the top left corner of the region grater than
     *                   nYmin. This defines the line range to be accessed.
     * \param [out] dpX
     * \param [out] dpY
     * \param [out] bufXSize
     * \param [out] bufYSize
     * \param [out] bufZmin
     * \param [out] bufZmax
     * \param [in] nXSize [optional] The size in pixels of the returned X dimension of the
     *                    region to be accessed. nXSize must be greater than nXmax-nXmin+1.
     *                    This parameter is mainly used for decimation in the X dimension.
     *                    If nXSize is not a multiple of nXmax-nXmin+1, due to the evenly
     *                    spaced decimation, the resulting nXmax may be greater than the
     *                    requested one.
     * \param [in] nYSize [optional] The size in pixels of the returned Y dimension of the
     *                    region to be accessed. nYSize must be greater than nYmax-nYmin+1.
     *                    This parameter is mainly used for decimation in the Y dimension.
     *                    If nYSize is not a multiple of nYmax-nYmin+1, due to the evenly
     *                    spaced decimation, the resulting nYmin may be greater than the
     *                    requested one.
     *
     * \return buffer2D
     */
    double **readInterpolateFromPixelsboundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                                  const double &pX1, const double &pY1,  // Lower-right corner
                                                  const std::size_t &nXsize, const std::size_t &nYsize);

    double **readInterpolateFromPixelsboundingBox(const double &pX0, const double &pY0,  // Upper-left corner
                                                  const double &pX1, const double &pY1,  // Lower-right corner
                                                  const std::size_t &nXsize, const std::size_t &nYsize,
                                                  double &zbufZmin, double &zbufZmax);

    //! readFromXYboundingBox
    /*! Read part of the Geotiff DEM into a 2D buffer array from natural coordinates
     *  bounding box, e.g., longitude/latitude or northing/easting.
     */
    double **readFromXYboundingBox(const double &Xmin, const double &Ymin,
                                   const double &Xmax, const double &Ymax,
                                   double &XminEx, double &YminEx,
                                   double &XmaxEx, double &YmaxEx,
                                   std::size_t &zbufXsize, std::size_t &zbufYsize);

    //! readFromXYboundingBox
    /*! Oveloaded function which computes the zbuffer min (zbufZmin) and max (zbufZmax)
     *  values taking into account the nodata values.
     */
    double **readFromXYboundingBox(const double &Xmin, const double &Ymin,
                                   const double &Xmax, const double &Ymax,
                                   double &XminEx, double &YminEx,
                                   double &XmaxEx, double &YmaxEx,
                                   std::size_t &zbufXsize, std::size_t &zbufYsize,
                                   double &zbufZmin, double &zbufZmax);

    double **readInterpolateFromXYboundingBox(const double &X0, const double &Y0,  // Upper-left corner
                                              const double &X1, const double &Y1,  // Lower-right corner
                                              const std::size_t &nXsize, const std::size_t &nYsize);

    double **readInterpolateFromXYboundingBox(const double &X0, const double &Y0,  // Upper-left corner
                                              const double &X1, const double &Y1,  // Lower-right corner
                                              const std::size_t &nXsize, const std::size_t &nYsize,
                                              double &zbufZmin, double &zbufZmax);

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

private:
    // ============== PRIVATE CLASS MEMBERS ==============
    std::filesystem::path      m_demPath;        // GeoTiff data path
    GDALDataset               *m_dataset;        // GeoTiff GDAL dataset
    GDALRasterBand            *m_rasterBand;     // GeoTiff Raster band
    OGRSpatialReference       *m_geoSpatialRef;  // GeoTiff Spatial Reference
    GeoTiffDEMAxes m_axes;
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
        // Get default block sizes
    std::size_t m_lineBufferSize;

    // ============== PRIVATE CLASS METHODS ==============
    void initializeGeoTiffDEM(std::filesystem::path demPath);
        //
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void fillZbuffer(const std::size_t &pXmin, const std::size_t &pYmin,
                     const std::size_t &zbufXsize, const std::size_t &zbufYsize,
                     double **zbuffer);
    // T -> type of the temporary line reading buffer, depending on raster band type
    // fillZbuffer: overloaded function which computes zmin and zmax while
    // filling z-buffer taking into account nodata values.
    template<typename T>
    void fillZbuffer(const std::size_t &pXmin, const std::size_t &pYmin,
                     const std::size_t &zbufXsize, const std::size_t &zbufYsize,
                     double **zbuffer, double &zbufZmin, double &zbufZmax);
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void fillZbuffer(const std::size_t &pXmin, const std::size_t &pYmin,
                     const std::size_t &bufXSize, const std::size_t &bufYSize,
                     const std::size_t &stepX, const std::size_t &stepY,
                     double **zbuffer, double &bufZmin, double &bufZmax);
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void interpolateFillZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                                const double &pX1, const double &pY1,  // Lower-right corner
                                const std::size_t &nXsize, const std::size_t &nYsize,
                                double **zbuffer);
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void interpolateFillZbuffer(const double &pX0, const double &pY0,  // Upper-left corner
                                const double &pX1, const double &pY1,  // Lower-right corner
                                const std::size_t &nXsize, const std::size_t &nYsize,
                                double **zbuffer, double &zbufZmin, double &zbufZmax);
};

#endif // GEOTIFFDEM_H
