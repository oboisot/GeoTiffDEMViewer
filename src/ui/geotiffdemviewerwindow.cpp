#include "geotiffdemviewerwindow.h"

GeoTiffDEMViewerWindow::GeoTiffDEMViewerWindow(QWidget *parent) : QMainWindow(parent)
{
    // QDEMColorMap
    createQDEMColorMap();
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
    QFile file(":/qss/dark/dark.qss");
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
    // Axes Unit default initialization
    m_axesUnit = GeoTiffDEMAxesUnit::Pixels;
    // DEM extent default initialization
    m_Xmin = m_Ymax = m_Xmax = m_Ymin=0.0;
    // QCustomPlot colors and style
    m_demCmap->setBackgroundColor(QColor(25, 25, 25));
    m_demCmap->setAxisRectBackgroundColor(QColor(80, 80, 80));
    m_demCmap->setAxesColor(QColor(169, 183, 198));
    // Signals and slots
    connect(m_demCmap, &QDEMColorMap::cursorChanged, this, [=](const QCursor &cursor){ this->setCursor(cursor); });
}

void GeoTiffDEMViewerWindow::createToolBar()
{
    QToolBar *toolBar = new QToolBar();
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, toolBar);
    // Open file action
    toolBar->addSeparator();
    QAction *openAction = new QAction(tr("&Open..."));
    openAction->setShortcut(QKeySequence::Open);
    openAction->setToolTip(QString("Open a GeoTiff DEM file (%1)").arg(QKeySequence(QKeySequence::Open).toString()));
    openAction->setIcon(QIcon(":/qss/dark/icons/svg/folder.svg"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(openDEMFile()));
    toolBar->addAction(openAction);
    openAction->setObjectName("openAction");
    toolBar->addSeparator();
    //
    QAction *resetZoomAction = new QAction("&Original view...");
    resetZoomAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    resetZoomAction->setToolTip(QString("Original view (%1)").arg(QKeySequence(Qt::CTRL | Qt::Key_0).toString()));
    resetZoomAction->setIcon(QIcon(":/qss/dark/icons/svg/zoom-original.svg"));
    connect(resetZoomAction, &QAction::triggered, this, [=](){ m_demCmap->resetZoom(); });
    toolBar->addAction(resetZoomAction);
    //
    QAction *zoomInAction = new QAction("&Zoom in...");
    zoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    zoomInAction->setToolTip(QString("Zoom in (%1)\nUse: mouse left double-click for X2 zoom out\nUse: Ctrl+<0-10> to go to the given zoom level").arg(
                                 QKeySequence(Qt::CTRL | Qt::Key_Plus).toString()));
    zoomInAction->setIcon(QIcon(":/qss/dark/icons/svg/zoom-in.svg"));
    connect(zoomInAction, &QAction::triggered, this, [=](){ m_demCmap->zoomIn(); });
    toolBar->addAction(zoomInAction);
    //
    QAction *zoomOutAction = new QAction("&Zoom out...");
    zoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    zoomOutAction->setToolTip(QString("Zoom out (%1)\nUse: mouse right double-click for X2 zoom out\nUse Ctrl+<0-10> to go to the given zoom level").arg(
                                  QKeySequence(Qt::CTRL | Qt::Key_Minus).toString()));
    zoomOutAction->setIcon(QIcon(":/qss/dark/icons/svg/zoom-out.svg"));
    connect(zoomOutAction, &QAction::triggered, this, [=](){ m_demCmap->zoomOut(); });
    toolBar->addAction(zoomOutAction);
    //
    QAction *refreshAction = new QAction("&Replot...");
    refreshAction->setShortcut(QKeySequence::Refresh);
    refreshAction->setToolTip(QString("Replot DEM (%1)").arg(QKeySequence(QKeySequence::Refresh).toString()));
    refreshAction->setIcon(QIcon(":/qss/dark/icons/svg/view-refresh.svg"));
//    connect(replotAction, &QAction::triggered, this, [=](){
//        if ( m_demCmap->isDEMOpened() )
//            m_demCmap->plotDEM(true);
//    });
    toolBar->addAction(refreshAction);
    toolBar->addSeparator();
    //
    QAction *selectionRectAction = new QAction("&Rect...");
    selectionRectAction->setShortcut(QKeySequence(tr("Ctrl+A")));
    selectionRectAction->setToolTip(QString("Selection area (Ctrl+A)"));
    selectionRectAction->setIcon(QIcon(":/qss/dark/icons/svg/selection-rect.svg"));
    toolBar->addAction(selectionRectAction);
    //
    QAction *saveAsBitmapAction = new QAction("&Save as...");
    saveAsBitmapAction->setShortcut(QKeySequence::SaveAs);
    saveAsBitmapAction->setToolTip(QString("Save current plot (%1)").arg(QKeySequence(QKeySequence::SaveAs).toString()));
    saveAsBitmapAction->setIcon(QIcon(":/qss/dark/icons/svg/document-save-as.svg"));
    toolBar->addAction(saveAsBitmapAction);
    // Information action
    QAction *infoAction = new QAction(tr("&Infos..."));
    infoAction->setShortcut(QKeySequence(tr("Ctrl+I")));
    infoAction->setToolTip(QString("DEM informations (Ctrl+I)"));
    infoAction->setIcon(QIcon(":/qss/dark/icons/svg/dialog-information.svg"));
    toolBar->addAction(infoAction);
    toolBar->addSeparator();
    // Get altitude Widget
    toolBar->addWidget(this->createGetAltWidget());
    toolBar->addSeparator();
    // Close application action
    QAction *closeAction = new QAction(tr("&Quit..."));
    closeAction->setShortcut(QKeySequence::Quit);
    closeAction->setToolTip(QString("Quit application (%1)").arg(QKeySequence(QKeySequence::Quit).toString()));
    closeAction->setIcon(QIcon(":/qss/dark/icons/svg/application-exit.svg"));
    connect(closeAction, &QAction::triggered, this, [&](){ this->close(); });
    toolBar->addAction(closeAction);
    toolBar->addSeparator();
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

QWidget *GeoTiffDEMViewerWindow::createGetAltWidget()
{
        // Labels
    m_Xlabel = new QLabel("X:");
    m_Xlabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_Ylabel = new QLabel("Y:");
    m_Ylabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_Zlabel = new QLabel(" → Alt: n/a");
    m_Zlabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_Zlabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_Xunit = new QLabel(" ");
    m_Xunit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_Yunit = new QLabel(" ");
    m_Yunit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        // Line edit validators
    m_XlineValidator = new QDoubleValidator(m_Xmin, m_Xmax, 12);
    m_XlineValidator->setLocale(m_numberLocale); // uses '.' as a decimal point
    m_YlineValidator = new QDoubleValidator(m_Ymin, m_Ymax, 12);
    m_YlineValidator->setLocale(m_numberLocale); // uses '.' as a decimal point
        // Line edit
    m_Xline = new QLineEdit();
    m_Xline->setPlaceholderText("");
    m_Xline->setValidator(m_XlineValidator);
    m_Xline->setMaximumWidth(100);
    m_Xline->setDisabled(true);
    m_Yline = new QLineEdit();
    m_Yline->setPlaceholderText("");
    m_Yline->setValidator(m_YlineValidator);
    m_Yline->setMaximumWidth(100);
    m_Yline->setDisabled(true);
    connect(m_Xline, &QLineEdit::textChanged, this, [=](){ this->XlineEditChanged(); });
    connect(m_Yline, &QLineEdit::textChanged, this, [=](){ this->YlineEditChanged(); });
        // Layout
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_Xlabel);
    layout->addWidget(m_Xline);
    layout->addWidget(m_Xunit);
    layout->addWidget(m_Ylabel);
    layout->addWidget(m_Yline);
    layout->addWidget(m_Yunit);
    layout->addWidget(m_Zlabel);
    layout->insertStretch(0, 1);
    layout->addStretch(1);
    QWidget *widget = new QWidget();
    widget->setLayout(layout);
    return widget;
}

void GeoTiffDEMViewerWindow::closeEvent(QCloseEvent *event)
{
    if (QMessageBox::Yes != QMessageBox::question(this, "Close Confirmation?",
        "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
    {
        event->ignore();
    }
}

/*****************
 * PRIVATE SLOTS *
 *****************/
void GeoTiffDEMViewerWindow::openDEMFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open a GeoTiff DEM file"),
                                                    "",
                                                    tr("GeoTIFF (*.tiff *.tif *.gtif)"));
    if ( fileName != "" )
    {
        fs::path demPath = fileName.toStdString();
        m_demCmap->openDEM(demPath);
        if ( m_demCmap->isDEMOpened() )
        {
            setWindowTitle(("GeoTiffDEM: " + demPath.filename().string()).c_str());
            m_demCmap->plotDEM(true);
            m_demCmap->getDEMExtent(m_Xmin, m_Ymax, m_Xmax, m_Ymin);
            m_XlineValidator->setBottom(m_Xmin);
            m_XlineValidator->setTop(m_Xmax);
            m_YlineValidator->setBottom(m_Ymin);
            m_YlineValidator->setTop(m_Ymax);
            m_Xline->setEnabled(true);
            m_Xline->setText("");
            m_Xline->setStyleSheet("QLineEdit {border: 1px solid gray;} QLineEdit:focus {border: 1px solid #d28140;}");
            m_Yline->setEnabled(true);
            m_Yline->setText("");
            m_Yline->setStyleSheet("QLineEdit {border: 1px solid gray;} QLineEdit:focus {border: 1px solid #d28140;}");
            m_Zlabel->setText(" → Alt: n/a");
            m_axesUnit = m_demCmap->getDEMAxesUnit();
            if ( m_axesUnit == GeoTiffDEMAxesUnit::LonLat )
            {
                m_Xlabel->setText("Lon:");
                m_Ylabel->setText("Lat:");
                m_Xunit->setText("°");
                m_Yunit->setText("°");
            }
            else if ( m_axesUnit == GeoTiffDEMAxesUnit::NorthEast )
            {
                m_Xlabel->setText("North:");
                m_Ylabel->setText("East:");
                m_Xunit->setText("m");
                m_Yunit->setText("m");
            }
            else
            {
                m_Xlabel->setText("X:");
                m_Ylabel->setText("Y:");
                m_Xunit->setText("px");
                m_Yunit->setText("px");
            }
        }
    }
}


/*******************
 * PRIVATE METHODS *
 *******************/
void GeoTiffDEMViewerWindow::XlineEditChanged()
{
    if ( m_demCmap->isDEMOpened() )
    {
        if ( m_Xline->hasAcceptableInput() )
        {
            m_Xline->setStyleSheet("QLineEdit {border: 1px solid gray;} QLineEdit:focus {border: 1px solid #d28140;}");
            double X = m_numberLocale.toDouble(m_Xline->text()),
                   Y = m_numberLocale.toDouble(m_Yline->text());
            m_Zlabel->setText(QString(" → Alt: %1").arg(m_demCmap->getZAtXYasStr(X, Y)));
        }
        else
            m_Xline->setStyleSheet("QLineEdit {border: 1px solid #ff0000;}");
    }
}

void GeoTiffDEMViewerWindow::YlineEditChanged()
{
    if ( m_demCmap->isDEMOpened() )
    {
        if ( m_Yline->hasAcceptableInput() )
        {
            m_Yline->setStyleSheet("QLineEdit {border: 1px solid gray;} QLineEdit:focus {border: 1px solid #d28140;}");
            double X = m_numberLocale.toDouble(m_Xline->text()),
                   Y = m_numberLocale.toDouble(m_Yline->text());
            m_Zlabel->setText(QString(" → Alt: %1").arg(m_demCmap->getZAtXYasStr(X, Y)));
        }
        else
            m_Yline->setStyleSheet("QLineEdit {border: 1px solid #ff0000;}");
    }
}
