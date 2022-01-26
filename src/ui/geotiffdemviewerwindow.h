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

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    // Define locale for numbers representation
    const QLocale m_numberLocale=QLocale::C;
    QDir currentDEMdir = QDir("");
    QDir currentSaveDir = QDir("");
    // DEM colormap
    QDEMColorMap *m_demCmap;
    GeoTiffDEMAxesUnit m_axesUnit;
    double m_Xmin, m_Ymax, m_Xmax, m_Ymin;
    // Toolbar widgets
    QLabel *m_tXlabel, *m_tYlabel, *m_tZlabel, *m_tXunit, *m_tYunit;
    QLineEdit *m_tXline, *m_tYline;
    QDoubleValidator *m_tXlineValidator, *m_tYlineValidator;
    // Status bar widgets
    QLabel *m_statusLabel, *m_coordlabel, *m_zoomLabel;
    QProgressBar *m_progressBar;
    QTimer *m_statusLabelTimer;
    // Infos dialog
    InfosDialog *m_infosDialog;
    // UI creation methods
    void createQDEMColorMap();
    void createToolBar();    
    void createStatusBar();
    void createInfosDialog();
    void createShortcuts();
    QWidget *createGetAltWidget();
    //
    void openDEMFile();
    void savePlot();
    void setStatusLabel(const QString &status, const QDEMStatusColor &color, const int &msec);
    void XlineTextChanged();
    void YlineTextChanged();
};

#endif // GEOTIFFDEMVIEWERWINDOW_H
