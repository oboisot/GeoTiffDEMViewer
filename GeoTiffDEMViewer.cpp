#include <iostream>
#include <filesystem>
#include <QApplication>
#include "geotiffdemviewerwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    GeoTiffDEMViewerWindow window;
    window.show();
    return app.exec();
}
