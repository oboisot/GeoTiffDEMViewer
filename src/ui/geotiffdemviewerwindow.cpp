#include "geotiffdemviewerwindow.h"

GeoTiffDEMViewerWindow::GeoTiffDEMViewerWindow(QWidget *parent) : QMainWindow(parent)
{
    // QDEMColorMap
    createQDEMColorMap();
    //MenuBar
//    createMenubar();
    // StatusBar
    createStatusBar();
    // ToolBar
    createToolBar();
    // Central Widget
    createCentralWidget();
    //
    setMouseTracking(true);
    setGeometry(600, 180, 720, 720);
    setMinimumSize(600, 600);
    setWindowTitle("GeoTiffDEM Viewer");
    //
    QFile file(":/qss/dark.qss");
    file.open(QFile::Text |QFile::ReadOnly);
    QString styleSheet = QString::fromUtf8(file.readAll());
    qApp->setStyleSheet(styleSheet);
}

GeoTiffDEMViewerWindow::~GeoTiffDEMViewerWindow()
{
    delete m_demCmap;
    GDALDestroyDriverManager();
}

//#######################################//
void GeoTiffDEMViewerWindow::createQDEMColorMap()
{
    // Create QDEMColorMap
    GDALAllRegister(); // Initialize GDAL drivers
    m_demCmap = new QDEMColorMap();
    // QCustomPlot colors and style
    m_demCmap->setBackgroundColor(QColor(25, 25, 25));
    m_demCmap->setAxisRectBackgroundColor(QColor(80, 80, 80));
    m_demCmap->setAxesColor(QColor(169, 183, 198));

    // Signals and slots
    connect(m_demCmap, &QDEMColorMap::plotChanged, this, [=](bool isPlotting){
        if ( isPlotting )
            setCursor(Qt::WaitCursor);
        else
            setCursor(Qt::ArrowCursor);
    });
}

void GeoTiffDEMViewerWindow::createMenubar()
{
//    // Open file QFfileDialog
//    m_openDialog = new QFileDialog(this);
//    m_openDialog->setWindowTitle(tr("Open a GeoTiff DEM file"));
//    m_openDialog->setAcceptMode(QFileDialog::AcceptOpen);
//    m_openDialog->setFileMode(QFileDialog::ExistingFile);
//    m_openDialog->setNameFilter(tr("GeoTIFF (*.tiff *.tif *.gtif)"));
//    // Open file action
//    QAction *openAction = new QAction(tr("&Open"));
//    connect(openAction, &QAction::triggered, this, [&](){
//        if ( m_openDialog->exec() )
//        {
//            QStringList fileList = m_openDialog->selectedFiles();
//            if ( !fileList.isEmpty() )
//            {
//                fs::path demPath = fileList[0].toStdString();
//                m_demCmap->openDEM(demPath);
//                setWindowTitle(("GeoTiffDEM: " + demPath.filename().string()).c_str());
//                m_demCmap->plotDEM(true);
//            }
//        }
//    });
    // Close application action
    QAction *closeAction = new QAction(tr("&Close"));
    connect(closeAction, &QAction::triggered, this, [&](){ this->close(); });
    // File menu
    QMenu *fileMenu = new QMenu(tr("&File"));
//    fileMenu->addAction(openAction);
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
    m_statusLabelTimer = new QTimer(this);
    m_statusLabel = new QLabel(" ");
    m_statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statusBar()->addPermanentWidget(m_statusLabel, 2);
    connect(m_demCmap, &QDEMColorMap::statusChanged, this,
            [=](const QString &status, const QDEMStatusColor &color, const int &msec)
    {
        m_statusLabelTimer->stop();
        m_statusLabel->setText(status);
        switch (color)
        {
        case Normal:
            m_statusLabel->setStyleSheet("QLabel { color : black; }");break;
        case Warning:
            m_statusLabel->setStyleSheet("QLabel { color : yellow; }");break;
        case Error:
            m_statusLabel->setStyleSheet("QLabel { color : red; }");break;
        default:
            m_statusLabel->setStyleSheet("QLabel { color : black; }");break;
        }
        if ( msec > 0 )
            m_statusLabelTimer->start(msec);
    });
    connect(m_statusLabelTimer, &QTimer::timeout, this, [=](){ m_statusLabel->setText(""); });
    //
    m_coordLabel = new QLabel(" ");
    m_coordLabel->setAlignment(Qt::AlignCenter);
    connect(m_demCmap, &QDEMColorMap::cmapCursorPosChanged, this,
        [=](const QString &value) { m_coordLabel->setText(value); }
    );
    statusBar()->addPermanentWidget(m_coordLabel, 3);
    //
    m_zoomLabel = new QLabel("zoom: 0");
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    connect(m_demCmap, &QDEMColorMap::zoomChanged, this,
        [=](const int &zoomLevel) { m_zoomLabel->setText(QString("zoom: %1").arg(zoomLevel)); }
    );
    statusBar()->addPermanentWidget(m_zoomLabel, 1);
    //
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    connect(m_demCmap, &QDEMColorMap::progressChanged, this,
        [=](const double &progress) { m_progressBar->setValue(static_cast<int>(progress * 100.0)); }
    );
    //
    statusBar()->addPermanentWidget(m_progressBar, 1);
}

void GeoTiffDEMViewerWindow::createToolBar()
{
    QToolBar *toolBar = new QToolBar();
    // Open file action
    QAction *openAction = new QAction(tr("&Open..."));
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setToolTip(tr("Open a GeoTiff DEM file (Ctrl+O)"));
    openAction->setIcon(QIcon(":/qss/dark/icons/svg/document-open.svg"));
    connect(openAction, &QAction::triggered, this, [=](){
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open a GeoTiff DEM file"),
                                                        "",
                                                        tr("GeoTIFF (*.tiff *.tif *.gtif)"));
        if ( fileName != "" )
        {
            fs::path demPath = fileName.toStdString();
            m_demCmap->openDEM(demPath);
            setWindowTitle(("GeoTiffDEM: " + demPath.filename().string()).c_str());
            m_demCmap->plotDEM(true);
        }
    });
    toolBar->addAction(openAction);
    openAction->setObjectName("openAction");
    toolBar->addSeparator();
    //
    QAction *resetZoomAction = new QAction("home");
    connect(resetZoomAction, &QAction::triggered, this, [=](){m_demCmap->resetZoom();});
    toolBar->addAction(resetZoomAction);
    //
    QAction *zoomPlusAction = new QAction("zoom+");
    connect(zoomPlusAction, &QAction::triggered, this, [=](){m_demCmap->zoomIn();});
    toolBar->addAction(zoomPlusAction);
    //
    QAction *zoomMinusAction = new QAction("zoom-");
    connect(zoomMinusAction, &QAction::triggered, this, [=](){m_demCmap->zoomOut();});
    toolBar->addAction(zoomMinusAction);
    //
    addToolBar(Qt::TopToolBarArea, toolBar);
}

void GeoTiffDEMViewerWindow::createCentralWidget()
{
//    QGridLayout *layout = new QGridLayout();
//    layout->setContentsMargins(0, 0, 0, 0);
//    QWidget *leftWidget = new QWidget();
//    layout->addWidget(leftWidget, 0, 0);
//    layout->addWidget(m_demCmap, 0, 1);
//    layout->setColumnStretch(0, 1);
//    layout->setColumnStretch(1, 3);
//    QWidget* centralWidget = new QWidget();
//    centralWidget->setLayout(layout);
//    setCentralWidget(centralWidget);
    setCentralWidget(m_demCmap);
}

void GeoTiffDEMViewerWindow::closeEvent(QCloseEvent *event)
{
    if (QMessageBox::Yes != QMessageBox::question(this, "Close Confirmation?",
        "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
    {
        event->ignore();
    }
}
