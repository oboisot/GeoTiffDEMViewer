#include "geotiffdemviewerwindow.h"

GeoTiffDEMViewerWindow::GeoTiffDEMViewerWindow(QWidget *parent) : QMainWindow(parent)
{
    //MenuBar
    QMenu *fileMenu = new QMenu(tr("&File"));
    fileMenu->addAction(tr("&Open"));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Close"));
    QMenu *editMenu = new QMenu(tr("&Edit"));
    editMenu->addAction(tr("&Open"));
    editMenu->addSeparator();
    editMenu->addAction(tr("&Close"));
    QMenuBar *menuBar = new QMenuBar();
    menuBar->addMenu(fileMenu);
    menuBar->addMenu(editMenu);
    this->setMenuBar(menuBar);

    this->setMouseTracking(true);
    this->setGeometry(0, 0, 720, 720);
    this->setWindowTitle("GeoTiffDEM");
    //

    // StatusBar
    this->statusBar()->showMessage("");

    //

    GDALAllRegister(); // Initialize GDAL drivers
    demCmap = new DEMColorMap();
    this->setCentralWidget(demCmap);
    this->connect(demCmap, SIGNAL(dataCursorChanged(const QString &)), this, SLOT(dataCursorChanged(const QString &)));
}

GeoTiffDEMViewerWindow::~GeoTiffDEMViewerWindow()
{
    delete demCmap;
    GDALDestroyDriverManager();
}

void GeoTiffDEMViewerWindow::dataCursorChanged(const QString &value)
{
    this->statusBar()->showMessage(value);
}

void GeoTiffDEMViewerWindow::closeEvent(QCloseEvent *event)
{
    if (QMessageBox::Yes != QMessageBox::question(this, "Close Confirmation?",
        "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
    {
        event->ignore();
    }
}
