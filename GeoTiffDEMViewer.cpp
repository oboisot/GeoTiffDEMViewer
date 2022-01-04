#include <iostream>
#include <filesystem>
#include <QApplication>
#include "geotiffdemviewerwindow.h"

namespace fs = std::filesystem;


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    fs::path demDir = "/media/oboisot/DATA/DEM/FRANCE";
//    fs::path demPath = demDir / "ALOSv31/AW3D30/AW3D30_N040W005-N055E010_WGS84-ELPS_FRANCE.tif";
    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_LAMB93-ORTHO_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ORTHO_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ELPS_FRANCE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_LAMB93-ORTHO_CORSE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ORTHO_CORSE.tif";
//    fs::path demPath = demDir / "RGEALTI/RGEALTI_2-0_5M_FRANCE_CORSE/RGEALTI_2-0_5M_WGS84-ELPS_CORSE.tif";

    // Set up Main window
    GeoTiffDEMViewerWindow window;
    window.show();
    window.demCmap->openDEM(demPath);
    window.demCmap->plotDEM();
    return app.exec();

}
