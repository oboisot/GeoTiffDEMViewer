#include "geotiffdemviewerwindow.h"

GeoTiffDEMViewerWindow::GeoTiffDEMViewerWindow(QWidget *parent) : QMainWindow(parent)
{
    // QDEMColorMap
    this->createQDEMColorMap();
    // StatusBar
    this->createStatusBar();
    // ToolBar
    this->createToolBar();
    // Central Widget
    this->setCentralWidget(m_demCmap);
    // Create infos dialog
    this->createInfosDialog();
    // Create keyboard zoom shortcuts
    this->createShortcuts();
    // Main window
    this->setGeometry(600, 180, 720, 720);
    this->setMinimumSize(720, 720);
    this->setWindowTitle("GeoTiff DEM Viewer");
    this->setWindowIcon(QIcon(":/qss/dark/icons/png@96x96/logo.png"));
    // Load stylesheet
    QFile file(":/qss/dark/dark.qss");
    file.open(QFile::Text|QFile::ReadOnly);
    QString styleSheet = QString::fromUtf8(file.readAll());
    qApp->setStyleSheet(styleSheet);
    qApp->setApplicationName("GeoTiff DEM Viewer");
}

GeoTiffDEMViewerWindow::~GeoTiffDEMViewerWindow()
{
    delete m_demCmap;
    GDALDestroyDriverManager();
}

//#######################################//
/**********
 * EVENTS *
 **********/
void GeoTiffDEMViewerWindow::closeEvent(QCloseEvent *event)
{
    if ( !m_demCmap->isDEMPlotting() )
        event->accept();
    else
    {
        event->ignore();
        this->setStatusLabel("Wait end of current plot progress before closing.", QDEMStatusColor::Warning, 2500);
    }
}

/*******************
 * PRIVATE METHODS *
 *******************/
void GeoTiffDEMViewerWindow::createQDEMColorMap()
{
    // Create QDEMColorMap
    GDALAllRegister(); // Initialize GDAL drivers
    m_demCmap = new QDEMColorMap();
    // Axes Unit default initialization
    m_axesUnit = GeoTiffDEMAxesUnit::Pixels;
    // DEM extent default initialization
    m_Xmin = m_Ymax = m_Xmax = m_Ymin = 0.0;
    // QCustomPlot colors and style
    m_demCmap->setBackgroundColor(QColor(25, 25, 25));
    m_demCmap->setAxisRectBackgroundColor(QColor(80, 80, 80));
    m_demCmap->setAxesColor(QColor(255, 255, 255));
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
    openAction->setIcon(QIcon(":/qss/dark/icons/svg@96x96/folder.svg"));    
    toolBar->addAction(openAction);
    openAction->setObjectName("openAction");
    // Save plot action
    QAction *savePlotAction = new QAction("&Save as...");
    savePlotAction->setShortcut(QKeySequence::Save);
    savePlotAction->setToolTip(QString("Save current plot (%1)").arg(QKeySequence(QKeySequence::Save).toString()));
    savePlotAction->setIcon(QIcon(":/qss/dark/icons/svg@96x96/document-save-as.svg"));
    toolBar->addAction(savePlotAction);
    // Information action
    QAction *infoAction = new QAction(tr("&Infos..."));
    infoAction->setShortcut(QKeySequence(tr("Ctrl+I")));
    infoAction->setToolTip(QString("DEM informations (Ctrl+I)"));
    infoAction->setIcon(QIcon(":/qss/dark/icons/svg@96x96/dialog-information.svg"));
    toolBar->addAction(infoAction);
    toolBar->addSeparator();
    // Reset zoom action
    QAction *resetZoomAction = new QAction("&Original view...");
    resetZoomAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    resetZoomAction->setToolTip(QString("Original view (%1)").arg(QKeySequence(Qt::CTRL | Qt::Key_0).toString()));
    resetZoomAction->setIcon(QIcon(":/qss/dark/icons/svg@96x96/zoom-original.svg"));
    toolBar->addAction(resetZoomAction);
    // Zoom in action
    QAction *zoomInAction = new QAction("&Zoom in...");
    zoomInAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    zoomInAction->setToolTip(QString("Zoom in (%1)\n"
                                     "Use: mouse left double-click for zoom in x2 at the click position\n"
                                     "Use: Ctrl+<0-9> to go to the given zoom level").arg(
                                 QKeySequence(Qt::CTRL | Qt::Key_Plus).toString()));
    zoomInAction->setIcon(QIcon(":/qss/dark/icons/svg@96x96/zoom-in.svg"));
    toolBar->addAction(zoomInAction);
    // Zoom out action
    QAction *zoomOutAction = new QAction("&Zoom out...");
    zoomOutAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    zoomOutAction->setToolTip(QString("Zoom out (%1)\n"
                                      "Use: mouse right double-click for zoom out x2 at the click position\n"
                                      "Use: Ctrl+<0-9> to go to the given zoom level").arg(
                                  QKeySequence(Qt::CTRL | Qt::Key_Minus).toString()));
    zoomOutAction->setIcon(QIcon(":/qss/dark/icons/svg@96x96/zoom-out.svg"));
    toolBar->addAction(zoomOutAction);
    // Selection rect action
    QAction *selectionRectAction = new QAction("&Rect...");
    selectionRectAction->setShortcut(QKeySequence(tr("Ctrl+R")));
    selectionRectAction->setToolTip(QString("Activate/Deactivate area selection (Ctrl+R)"));
    selectionRectAction->setIcon(QIcon(":/qss/dark/icons/svg@96x96/selection-rect.svg"));
    selectionRectAction->setCheckable(true);
    selectionRectAction->setChecked(false);
    toolBar->addAction(selectionRectAction);
    toolBar->addSeparator();
    // Get altitude Widget
    toolBar->addWidget(this->createGetAltWidget());
    toolBar->addSeparator();
    // Close application action
    QAction *closeAction = new QAction(tr("&Quit..."));
    closeAction->setShortcut(QKeySequence::Quit);
    closeAction->setToolTip(QString("Quit application (%1)").arg(QKeySequence(QKeySequence::Quit).toString()));
    closeAction->setIcon(QIcon(":/qss/dark/icons/svg@96x96/application-exit.svg"));
    toolBar->addAction(closeAction);
    toolBar->addSeparator();
    toolBar->setContextMenuPolicy(Qt::NoContextMenu);
    // Signals and slots
        // Open file action
    connect(openAction, &QAction::triggered, this, [=](){ this->openDEMFile(); });
        // Save plot action
    connect(savePlotAction, &QAction::triggered, this, [=](){ if ( m_demCmap->isDEMOpened() ) this->savePlot(); });
        // Information action
    connect(infoAction, &QAction::triggered, this, [&](){
        QString infos(m_demCmap->getDEMinfos());
        m_infosDialog->setText((infos == "") ? "No DEM opened." : infos);
        m_infosDialog->open();
    });
        // Reset zoom action
    connect(resetZoomAction, &QAction::triggered, this, [=](){ m_demCmap->resetZoom(); });
        // Zoom in action
    connect(zoomInAction, &QAction::triggered, this, [=](){ m_demCmap->zoomIn(); });
        // Zoom out action
    connect(zoomOutAction, &QAction::triggered, this, [=](){ m_demCmap->zoomOut(); });
        // Selection rect action
        // Note: the stylesheet is handled here, the size of the toolbutton associated to a checkable qaction changes when clicked,
        //       this case is not handleable with the stylesheet directly... (the :unchecked or :off pseudo-states have no effects)
    QToolButton *selectionRectToolButton = qobject_cast<QToolButton*>(toolBar->widgetForAction(selectionRectAction));
    selectionRectToolButton->setObjectName("selectionRectToolButton");
    connect(selectionRectAction, &QAction::toggled, this, [=](){
        if ( selectionRectAction->isChecked() )
        {
            m_demCmap->selectionRectEnabled(true);
            selectionRectToolButton->setStyleSheet("QToolButton { min-height: 32px; min-width: 32px; border: 2px solid #ffffff; background-color: rgba(255,255,255,100); }"
                                                   "QToolButton:pressed { background-color: rgba(255,255,255,150); }");
            this->setStatusLabel("Area selection activated", QDEMStatusColor::Normal, 2500);
        }
        else
        {
            m_demCmap->selectionRectEnabled(false);
            selectionRectToolButton->setStyleSheet("QToolButton { min-height: 34px; min-width: 34px; border: none; background-color: #191919; margin: 0px; }"
                                                   "QToolButton:hover { border: 1px solid #ffffff; }"
                                                   "QToolButton:pressed { border: 2px solid #ffffff; background-color: rgba(100,100,100,120); }");
            this->setStatusLabel("Area selection deactivated", QDEMStatusColor::Normal, 2500);
        }
    });
        // Close application action
    connect(closeAction, &QAction::triggered, this, [=](){ this->close(); });
}

void GeoTiffDEMViewerWindow::createStatusBar()
{
    //
    m_statusLabelTimer = new QTimer(this);
    m_statusLabel = new QLabel(" ");
    m_statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    this->statusBar()->addPermanentWidget(m_statusLabel, 2);
    // Coordinates display
    m_coordlabel = new QLabel(" ");
    m_coordlabel->setAlignment(Qt::AlignCenter);
    this->statusBar()->addPermanentWidget(m_coordlabel, 3);
    //
    m_zoomLabel = new QLabel("zoom: 0");
    m_zoomLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);    
    this->statusBar()->addPermanentWidget(m_zoomLabel, 1);
    //
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    this->statusBar()->addPermanentWidget(m_progressBar, 1);
    // Signals and slots
        // Status label
    connect(m_demCmap, &QDEMColorMap::statusChanged, this, [=](const QString &status, const QDEMStatusColor &color, const int &msec) { this->setStatusLabel(status, color, msec); });
    connect(m_statusLabelTimer, &QTimer::timeout, this, [=](){ m_statusLabel->setText(""); });
        // Coordinates display
    connect(m_demCmap, &QDEMColorMap::cmapCursorPosChanged, this, [=](const QString &value) { m_coordlabel->setText(value); });
        // Zoom label
    connect(m_demCmap, &QDEMColorMap::zoomChanged, this, [=](const int &zoomLevel) { m_zoomLabel->setText(QString("zoom: %1").arg(zoomLevel)); });
        // Progress bar
    connect(m_demCmap, &QDEMColorMap::progressChanged, this, [=](const double &progress) { m_progressBar->setValue(static_cast<int>(progress * 100.0)); });
}

void GeoTiffDEMViewerWindow::createInfosDialog()
{
    m_infosDialog = new InfosDialog(this);
    m_infosDialog->setMinimumSize(720, 600);
}

void GeoTiffDEMViewerWindow::createShortcuts()
{
    QShortcut *ctrl1 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_1), this);
    connect(ctrl1, &QShortcut::activated, this, [=](){ m_demCmap->zoomTo(1); });
    QShortcut *ctrl2 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_2), this);
    connect(ctrl2, &QShortcut::activated, this, [=](){ m_demCmap->zoomTo(2); });
    QShortcut *ctrl3 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_3), this);
    connect(ctrl3, &QShortcut::activated, this, [=](){ m_demCmap->zoomTo(3); });
    QShortcut *ctrl4 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_4), this);
    connect(ctrl4, &QShortcut::activated, this, [=](){ m_demCmap->zoomTo(4); });
    QShortcut *ctrl5 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_5), this);
    connect(ctrl5, &QShortcut::activated, this, [=](){ m_demCmap->zoomTo(5); });
    QShortcut *ctrl6 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_6), this);
    connect(ctrl6, &QShortcut::activated, this, [=](){ m_demCmap->zoomTo(6); });
    QShortcut *ctrl7 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_7), this);
    connect(ctrl7, &QShortcut::activated, this, [=](){ m_demCmap->zoomTo(7); });
    QShortcut *ctrl8 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_8), this);
    connect(ctrl8, &QShortcut::activated, this, [=](){ m_demCmap->zoomTo(8); });
    QShortcut *ctrl9 = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_9), this);
    connect(ctrl9, &QShortcut::activated, this, [=](){ m_demCmap->zoomTo(9); });
}

QWidget *GeoTiffDEMViewerWindow::createGetAltWidget()
{
        // Labels
    m_tXlabel = new QLabel("X:");
    m_tXlabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_tYlabel = new QLabel("Y:");
    m_tYlabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_tZlabel = new QLabel(" → Alt: n/a");
    m_tZlabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_tZlabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_tXunit = new QLabel(" ");
    m_tXunit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_tYunit = new QLabel(" ");
    m_tYunit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        // Line edit validators
    m_tXlineValidator = new QDoubleValidator(m_Xmin, m_Xmax, 12);
    m_tXlineValidator->setLocale(m_numberLocale); // uses '.' as a decimal point
    m_tYlineValidator = new QDoubleValidator(m_Ymin, m_Ymax, 12);
    m_tYlineValidator->setLocale(m_numberLocale); // uses '.' as a decimal point
        // Line edit
    m_tXline = new QLineEdit();
    m_tXline->setPlaceholderText("");
    m_tXline->setValidator(m_tXlineValidator);
    m_tXline->setMaximumWidth(100);
    m_tXline->setDisabled(true);
    m_tYline = new QLineEdit();
    m_tYline->setPlaceholderText("");
    m_tYline->setValidator(m_tYlineValidator);
    m_tYline->setMaximumWidth(100);
    m_tYline->setDisabled(true);
        // Layout
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_tXlabel);
    layout->addWidget(m_tXline);
    layout->addWidget(m_tXunit);
    layout->addWidget(m_tYlabel);
    layout->addWidget(m_tYline);
    layout->addWidget(m_tYunit);
    layout->addWidget(m_tZlabel);
    layout->insertStretch(0, 1);
    layout->addStretch(1);
    QWidget *widget = new QWidget();
    widget->setLayout(layout);
    // Signals and slots
    connect(m_tXline, &QLineEdit::textChanged, this, [=](){ this->XlineTextChanged(); });
    connect(m_tXline, &QLineEdit::returnPressed, this, [=](){ this->XlineTextChanged(); });
    connect(m_tYline, &QLineEdit::textChanged, this, [=](){ this->YlineTextChanged(); });
    connect(m_tYline, &QLineEdit::returnPressed, this, [=](){ this->YlineTextChanged(); });
    return widget;
}

void GeoTiffDEMViewerWindow::openDEMFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open a GeoTiff DEM file"),
                                                    currentDEMdir.absolutePath(),
                                                    tr("GeoTIFF (*.tiff *.tif *.gtif)"));
    if ( fileName != "" )
    {
        if ( !m_demCmap->isDEMPlotting())
        {
            fs::path demPath = fileName.toStdString();
            m_demCmap->openDEM(demPath);
            if ( m_demCmap->isDEMOpened() )
            {
                this->setWindowTitle(("GeoTiff DEM Viewer: " + demPath.filename().string()).c_str());
                m_demCmap->plotDEM(true);
                m_demCmap->getDEMExtent(m_Xmin, m_Ymax, m_Xmax, m_Ymin);
                // Toolbar coordinates widget
                m_tXlineValidator->setBottom(m_Xmin);
                m_tXlineValidator->setTop(m_Xmax);
                m_tYlineValidator->setBottom(m_Ymin);
                m_tYlineValidator->setTop(m_Ymax);
                m_tXline->setEnabled(true);
                m_tXline->setText("");
                m_tXline->setStyleSheet("QLineEdit {border: 1px solid gray;} QLineEdit:focus {border: 1px solid #d28140;}");
                m_tYline->setEnabled(true);
                m_tYline->setText("");
                m_tYline->setStyleSheet("QLineEdit {border: 1px solid gray;} QLineEdit:focus {border: 1px solid #d28140;}");
                m_tZlabel->setText(" → Alt: n/a");
                // Statusbar coordinates widget
                m_axesUnit = m_demCmap->getDEMAxesUnit();
                if ( m_axesUnit == GeoTiffDEMAxesUnit::LonLat )
                {
                    // Toolbar coordinates widget
                    m_tXlabel->setText("Lon:");
                    m_tYlabel->setText("Lat:");
                    m_tXunit->setText("°");
                    m_tYunit->setText("°");
                }
                else if ( m_axesUnit == GeoTiffDEMAxesUnit::EastNorth )
                {
                    // Toolbar coordinates widget
                    m_tXlabel->setText("East:");
                    m_tYlabel->setText("North:");
                    m_tXunit->setText("m");
                    m_tYunit->setText("m");
                }
                else
                {
                    // Toolbar coordinates widget
                    m_tXlabel->setText("X:");
                    m_tYlabel->setText("Y:");
                    m_tXunit->setText("px");
                    m_tYunit->setText("px");
                }
                QFileInfo fileInfo(fileName);
                if ( fileInfo.absoluteDir() != currentDEMdir )
                    currentDEMdir = fileInfo.absoluteDir();
            }
            else
                this->setStatusLabel("Error when opening DEM.", QDEMStatusColor::Error, 2500);
        }
        else
            this->setStatusLabel("Wait end of current plot progress.", QDEMStatusColor::Warning, 2500);
    }
}

void GeoTiffDEMViewerWindow::savePlot()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save current plot"),
                                                    currentSaveDir.filePath("untitled.png"),
                                                    tr("Bitmap (*.png *.jpg);; PDF (*.pdf); "));
    if ( fileName != "" )
    {
        if ( !m_demCmap->isDEMPlotting())
        {
            QFileInfo fileInfo(fileName);
            QString ext = fileInfo.suffix();
            m_demCmap->setGeolocationCursorVisibility(false);
            if ( ext == "png" )
            {
                m_demCmap->savePng(fileName);
                this->setStatusLabel(QString("%1 saved.").arg(fileInfo.fileName()), QDEMStatusColor::Normal, 2500);
            }
            else if ( ext == "jpg" )
            {
                m_demCmap->saveJpg(fileName);
                this->setStatusLabel(QString("%1 saved.").arg(fileInfo.fileName()), QDEMStatusColor::Normal, 2500);
            }
            else if ( ext == "pdf" )
            {
                m_demCmap->savePdf(fileName);
                this->setStatusLabel(QString("%1 saved.").arg(fileInfo.fileName()), QDEMStatusColor::Normal, 2500);
            }
            else
                this->setStatusLabel("No plot saved.", QDEMStatusColor::Warning, 2500);
            m_demCmap->setGeolocationCursorVisibility(true);
            if ( fileInfo.absoluteDir() != currentSaveDir )
                currentSaveDir = fileInfo.absoluteDir();
        }
        else
            this->setStatusLabel("Wait end of current plot progress.", QDEMStatusColor::Warning, 2500);
    }
}

void GeoTiffDEMViewerWindow::setStatusLabel(const QString &status, const QDEMStatusColor &color, const int &msec)
{
    m_statusLabelTimer->stop();
    m_statusLabel->setText(status);
    switch (color)
    {
    case Normal:
        m_statusLabel->setStyleSheet("QLabel { color : white; }");break;
    case Warning:
        m_statusLabel->setStyleSheet("QLabel { color : yellow; }");break;
    case Error:
        m_statusLabel->setStyleSheet("QLabel { color : red; }");break;
    default:
        m_statusLabel->setStyleSheet("QLabel { color : white; }");break;
    }
    if ( msec > 0 ) m_statusLabelTimer->start(msec);
}

void GeoTiffDEMViewerWindow::XlineTextChanged()
{
    if ( m_demCmap->isDEMOpened() && !m_demCmap->isDEMPlotting() )
    {
        if ( m_tXline->hasAcceptableInput() )
        {
            m_tXline->setStyleSheet("QLineEdit {border: 1px solid gray;} QLineEdit:focus {border: 1px solid #d28140;}");
            double X = m_numberLocale.toDouble(m_tXline->text()),
                   Y = m_numberLocale.toDouble(m_tYline->text());
            m_tZlabel->setText(QString(" → Alt: %1").arg(m_demCmap->getZAtXYasStr(X, Y)));
            m_demCmap->setGeolocationCursorPosition(X, Y);
        }
        else
            m_tXline->setStyleSheet("QLineEdit {border: 1px solid #ff0000;}");
    }
}

void GeoTiffDEMViewerWindow::YlineTextChanged()
{
    if ( m_demCmap->isDEMOpened() && !m_demCmap->isDEMPlotting() )
    {
        if ( m_tYline->hasAcceptableInput() )
        {
            m_tYline->setStyleSheet("QLineEdit {border: 1px solid gray;} QLineEdit:focus {border: 1px solid #d28140;}");
            double X = m_numberLocale.toDouble(m_tXline->text()),
                   Y = m_numberLocale.toDouble(m_tYline->text());
            m_tZlabel->setText(QString(" → Alt: %1").arg(m_demCmap->getZAtXYasStr(X, Y)));
            m_demCmap->setGeolocationCursorPosition(X, Y);
        }
        else
            m_tYline->setStyleSheet("QLineEdit {border: 1px solid #ff0000;}");
    }
}
