#ifndef GEOTIFFDEMVIEWERWINDOW_H
#define GEOTIFFDEMVIEWERWINDOW_H

#include <iostream>
#include <QMainWindow>
#include <QTimer>
#include "qdemcolormap.h"
#include "infosdialog.h"

class GeoTiffDEMViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GeoTiffDEMViewerWindow(QWidget *parent = nullptr);
    ~GeoTiffDEMViewerWindow();

private slots:
    void openDEMFile();

private:
    // Define locale for numbers representation
    const QLocale m_numberLocale=QLocale::C;
    //
    QDEMColorMap *m_demCmap;
    GeoTiffDEMAxesUnit m_axesUnit;
    double m_Xmin, m_Ymax, m_Xmax, m_Ymin;
    // Toolbar widgets
    QDoubleValidator *m_XlineValidator, *m_YlineValidator;
    QLabel *m_Xlabel, *m_Ylabel, *m_Zlabel, *m_Xunit, *m_Yunit;
    QLineEdit *m_Xline, *m_Yline;
    // Status bar QWidgets
    QLabel *m_statusLabel, *m_coordLabel, *m_zoomLabel;
    QProgressBar *m_progressBar;
    QTimer *m_statusLabelTimer;
    // Infos dialog
    InfosDialog *m_infosDialog;
    //
    void createQDEMColorMap();
    void createToolBar();    
    void createStatusBar();
    void createCentralWidget();
    void createInfosDialog();
    void createShortcuts();    
    QWidget *createGetAltWidget();
    //
    void XlineTextChanged();
    void YlineTextChanged();
};

#endif // GEOTIFFDEMVIEWERWINDOW_H
