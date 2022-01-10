#ifndef GEOTIFFDEMVIEWERWINDOW_H
#define GEOTIFFDEMVIEWERWINDOW_H

#include <iostream>
#include <QMainWindow>
#include "qdemcolormap.h"

class GeoTiffDEMViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GeoTiffDEMViewerWindow(QWidget *parent = nullptr);
    ~GeoTiffDEMViewerWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    //
    QDEMColorMap *m_demCmap;
    // Menubar QWidgets
    QFileDialog *m_openDialog;
    // Status bar QWidgets
    QLabel *m_infosLabel;
    QLabel *m_coordLabel;
    QProgressBar *m_progressBar;
    //
    void createDEMCmap();
    void createMenubar();
    void createStatusBar();
    void createToolBar();
    void createQDEMColorMap();
};

#endif // GEOTIFFDEMVIEWERWINDOW_H
