#include <iostream>
#include <filesystem>
//#include <limits>
//#include "gdal_priv.h"
//#include "cpl_conv.h" // for CPLMalloc()

//#include <QApplication>
//#include "mainwindow.h"

#include "src/geotiffdem.h"

namespace fs = std::filesystem;


//int main(int argc, char *argv[])
int main()
{
    GDALAllRegister(); // Initialize GDAL drivers
    // "This function should generally be called once at the beginning of the application."

    fs::path demDir = "/media/oboisot/DATA/DEM/FRANCE";
    fs::path demPath = demDir / "ALOSv31/AW3D30/AW3D30_N040W005-N055E010_WGS84-ELPS_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_LAMB93-ORTHO_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ORTHO_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ELPS_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_LAMB93-ORTHO_CORSE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ORTHO_CORSE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ELPS_CORSE.tif";


    GeoTiffDEM *dem = new GeoTiffDEM( demPath.c_str() );

    std::cout << std::setprecision(14);
    std::cout << "rasterXSize = " << dem->getRasterXSize() << std::endl;
    std::cout << "rasterYSize = " << dem->getRasterYSize() << std::endl;
    std::cout << "lonmin = " << dem->getXmin() << std::endl;
    std::cout << "latmax = " << dem->getYmax() << std::endl;
    std::cout << "dlon = " << dem->getdX() << std::endl;
    std::cout << "dlat = " << dem->getdY() << std::endl;
//    dem->printPrettySpatialRef();


//    const std::size_t pXmin = 0,//dem->getRasterXSize() / 2,
//                      pYmin = 0,//dem->getRasterYSize() / 2;
//                      pXmax = dem->getRasterXSize() - 1,
//                      pYmax = dem->getRasterYSize() - 1;
//    std::size_t dpX, dpY, bufXSize, bufYSize;
//    double Altmin, Altmax,
//           **zbuffer = dem->readFromPixelsboundingBox(pXmin, pYmin, pXmax, pYmax,
//                                                      dpX, dpY, bufXSize, bufYSize,
//                                                      Altmin, Altmax, 1000, 1000);
//    dem->deleteZbuffer(bufYSize, zbuffer);

//    double Xmin = 2,
//           Ymin = 43,
//           Xmax = 2.1,
//           Ymax = 43.1;
//    std::size_t bufXSize, bufYSize;
//    double XminEx, YminEx, XmaxEx, YmaxEx, Zmin, Zmax,
//           **zbuffer = dem->readFromXYboundingBox(Xmin, Ymin, Xmax, Ymax,
//                                                  XminEx, YminEx, XmaxEx, YmaxEx,
//                                                  bufXSize, bufYSize, Zmin, Zmax);
//    dem->deleteZbuffer(bufYSize, zbuffer);

//    std::cout << "bufXSize = " << bufXSize << std::endl;
//    std::cout << "bufYSize = " << bufYSize << std::endl;
//    std::cout << "XminEx = " << XminEx << std::endl;
//    std::cout << "YminEx = " << YminEx << std::endl;
//    std::cout << "XmaxEx = " << XmaxEx << std::endl;
//    std::cout << "YmaxEx = " << YmaxEx << std::endl;
//    std::cout << "Zmin = " << Zmin << std::endl;
//    std::cout << "Zmax = " << Zmax << std::endl;

    double lonmin = dem->getXmin(),
           lonmax = dem->getXmax(),
           latmin = dem->getYmin(),
           latmax = dem->getYmax();
    std::size_t xsize = 2500,
                ysize = 2500;
    double zbufZmin, zbufZmax;
    double **lonlat = dem->readInterpolateFromXYboundingBox(lonmin, latmin, lonmax, latmax,
                                                            xsize, ysize, zbufZmin, zbufZmax);
    std::cout << "zbufZmin = " << zbufZmin << std::endl;
    std::cout << "zbufZmax = " << zbufZmax << std::endl;
//    for ( std::size_t ilat = 0 ; ilat < latsize ; ++ilat)
//    {
//        for ( std::size_t ilon = 0 ; ilon < lonsize ; ++ilon )
//        {
//            std::cout << "lonlat[" << ilat << "][" << ilon << "] = " << lonlat2[ilat][ilon] << std::endl;
//        }
//    }
    dem->deleteZbuffer(ysize, lonlat);

//    std::size_t xsize = 10000, ysize = 10000;
//    double **lonlat2 = dem->readInterpolateFromPixelsboundingBox(0, 0, 10, 10,
//                                                                 xsize, ysize);
////    for ( std::size_t iY = 0 ; iY < 10 ; ++iY )
////        for ( std::size_t iX = 0 ; iX < 10 ; ++iX )
////            std::cout << "lonlat2[" << iY << "][" << iX << "] = " << lonlat2[iY][iX] << std::endl;
//    dem->deleteZbuffer(ysize, lonlat2);
    delete dem;
    GDALDestroyDriverManager();
    return 0;
}
