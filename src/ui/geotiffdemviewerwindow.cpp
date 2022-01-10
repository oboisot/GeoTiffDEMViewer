#include "geotiffdemviewerwindow.h"

GeoTiffDEMViewerWindow::GeoTiffDEMViewerWindow(QWidget *parent) : QMainWindow(parent)
{
    // QDEMColorMap
    createDEMCmap();
    //MenuBar
    createMenubar();
    // StatusBar
    createStatusBar();
    // ToolBar
    createToolBar();
    //
    setCentralWidget(m_demCmap);
    setMouseTracking(true);
    setGeometry(600, 180, 720, 720);
    setWindowTitle("GeoTiffDEM");
}

GeoTiffDEMViewerWindow::~GeoTiffDEMViewerWindow()
{
    delete m_demCmap;
    GDALDestroyDriverManager();
}

//#######################################//
void GeoTiffDEMViewerWindow::createDEMCmap()
{
    GDALAllRegister(); // Initialize GDAL drivers
    m_demCmap = new QDEMColorMap();
}

void GeoTiffDEMViewerWindow::createMenubar()
{
    // Open file QFfileDialog
    m_openDialog = new QFileDialog();
    m_openDialog->setWindowTitle(tr("Open a GeoTiff DEM file"));
    m_openDialog->setAcceptMode(QFileDialog::AcceptOpen);
    m_openDialog->setFileMode(QFileDialog::ExistingFile);
    m_openDialog->setNameFilter(tr("GeoTIFF (*.tiff *.tif *.gtif)"));
    // Open file action
    QAction *openAction = new QAction(tr("&Open"));
    connect(openAction, &QAction::triggered, this, [&](){
        if ( m_openDialog->exec() )
        {
            QStringList fileList = m_openDialog->selectedFiles();
            if ( !fileList.isEmpty() )
            {
                fs::path demPath = fileList[0].toStdString();
                m_demCmap->openDEM(demPath);
                setWindowTitle(("GeoTiffDEM: " + demPath.filename().string()).c_str());
                m_demCmap->plotDEM(true);
            }
        }
    });
    // Close application action
    QAction *closeAction = new QAction(tr("&Close"));
    connect(closeAction, &QAction::triggered, this, [&](){ this->close(); });
    // File menu
    QMenu *fileMenu = new QMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAction);
    // MenuBar
    QMenuBar *menuBar = new QMenuBar();
    menuBar->addMenu(fileMenu);
    setMenuBar(menuBar);
}

void GeoTiffDEMViewerWindow::createStatusBar()
{
    //
    m_infosLabel = new QLabel(" ");
    m_infosLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statusBar()->addPermanentWidget(m_infosLabel, 2);
    //
    m_coordLabel = new QLabel(" ");
    m_coordLabel->setAlignment(Qt::AlignCenter);
    connect(m_demCmap, &QDEMColorMap::cmapCursorPosChanged, this,
        [=](const QString &value) { m_coordLabel->setText(value); }
    );
    statusBar()->addPermanentWidget(m_coordLabel, 3);
    //
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    connect(m_demCmap, &QDEMColorMap::progressChanged, this,
        [&](const double &progress) { m_progressBar->setValue(static_cast<int>(progress * 100.0)); }
    );
    //
    this->statusBar()->addPermanentWidget(m_progressBar, 1);
}

void GeoTiffDEMViewerWindow::createToolBar()
{
    QToolBar *toolBar = new QToolBar();
    //
    QAction *resetZoomAction = new QAction("home");
    connect(resetZoomAction, &QAction::triggered, this, [&](){m_demCmap->resetZoom();});
    toolBar->addAction(resetZoomAction);
    //
    QAction *zoomPlusAction = new QAction("zoom+");
    connect(zoomPlusAction, &QAction::triggered, this, [&](){m_demCmap->zoomIn();});
    toolBar->addAction(zoomPlusAction);
    //
    QAction *zoomMinusAction = new QAction("zoom-");
    connect(zoomMinusAction, &QAction::triggered, this, [&](){m_demCmap->zoomOut();});
    toolBar->addAction(zoomMinusAction);
    //
    addToolBar(Qt::TopToolBarArea, toolBar);
}

void GeoTiffDEMViewerWindow::closeEvent(QCloseEvent *event)
{
    if (QMessageBox::Yes != QMessageBox::question(this, "Close Confirmation?",
        "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
    {
        event->ignore();
    }
}
