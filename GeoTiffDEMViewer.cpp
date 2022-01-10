#include <iostream>
#include <filesystem>
#include <QApplication>
#include "geotiffdemviewerwindow.h"
//#include "src/geotiffdem.h"

namespace fs = std::filesystem;


int main(int argc, char *argv[])
{
    fs::path demDir = "/media/oboisot/DATA/DEM/FRANCE";
    fs::path demPath = demDir / "ALOSv31/AW3D30/AW3D30_N040W005-N055E010_WGS84-ELPS_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_LAMB93-ORTHO_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ORTHO_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ELPS_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_LAMB93-ORTHO_CORSE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ORTHO_CORSE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ELPS_CORSE.tif";

//    GeoTiffDEM *dem = new GeoTiffDEM();
//    dem->initDrivers();
//    dem->open(demPath);

//    double Xmin = 5.007244,
//           Ymax = 43.83799,
//           Xmax = 5.127657,
//           Ymin = 43.772272;
//    std::size_t bufXsize = 10, bufYsize = 1;
//    double **zbuffer = dem->interpLinFromXYboundingBox(Xmin, Ymin, Xmax, Ymin, bufXsize, bufYsize);

//    std::cout << sizeof(double) * bufXsize * bufYsize << std::endl;
//    for (std::size_t iY = 0 ; iY < bufYsize ; ++iY)
//        for (std::size_t iX = 0 ; iX < bufXsize ; ++iX)
//            std::cout << "zbuffer[" << iY << "][" << iX << "] = " << zbuffer[iY][iX] << std::endl;
//    std::cout << dem->getZAtXY(Xmin, Ymin) << std::endl;

//    dem->deleteZbuffer(bufYsize, zbuffer);
//    dem->destroyDrivers();
////    delete dem;
//    return 0;
    QApplication app(argc, argv);
    // Set up Main window
    GeoTiffDEMViewerWindow window;
    window.show();
//    window.demCmap->openDEM(demPath);
//    window.demCmap->plotDEM();
    return app.exec();
}
