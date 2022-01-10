#ifndef QGEOTIFFDEM_H
#define QGEOTIFFDEM_H

#include "qcustomplot.h"
#include "geotiffdem.h"

class QGeoTiffDEM : public QObject, public GeoTiffDEM
{
    Q_OBJECT

public:
    /*! ********************************************************************
    * \brief QGeoTiffDEM
    *********************************************************************/
    explicit QGeoTiffDEM(QObject *parent = nullptr);

    // ============== DESTRUCTOR ==============
    //! Default destructor
    ~QGeoTiffDEM();

    void interpFromPixelsBBoxToQCPColorMapData(const double &pX0, const double &pY0,  // Upper-left corner
                                               const double &pX1, const double &pY1,  // Lower-right corner
                                               const std::size_t &nXsize, const std::size_t &nYsize,
                                               double &zbufZmin, double &zbufZmax,
                                               QCPColorMapData *data,
                                               GeoTiffDEMinterp interp=Nearest);

    void interpFromXYBBoxToQCPColorMapData(const double &Xmin, const double &Ymax,  // Upper-left corner
                                           const double &Xmax, const double &Ymin,  // Lower-right corner
                                           const std::size_t &nXsize, const std::size_t &nYsize,
                                           double &zbufZmin, double &zbufZmax,
                                           QCPColorMapData *data,
                                           GeoTiffDEMinterp interp=Nearest);

signals:
    void progressChanged(const double &progress);

private:
    // ============== PRIVATE CLASS METHODS ==============
    // Nearest-Neighbor interpolation on pixels
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void interpNNToQCPColorMapData(const double &pX0, const double &pY0,  // Upper-left corner
                                   const double &pX1, const double &pY1,  // Lower-right corner
                                   const std::size_t &nXsize, const std::size_t &nYsize,
                                   double &zbufZmin, double &zbufZmax,
                                   QCPColorMapData *data);
    // Linear interpolation on pixels
    // T -> type of the temporary line reading buffer, depending on raster band type
    template<typename T>
    void interpLinToQCPColorMapData(const double &pX0, const double &pY0,  // Upper-left corner
                                   const double &pX1, const double &pY1,  // Lower-right corner
                                   const std::size_t &nXsize, const std::size_t &nYsize,
                                   double &zbufZmin, double &zbufZmax,
                                   QCPColorMapData *data);
};

#endif // QGEOTIFFDEM_H
