#ifndef GEOTIFFDEMVIEWERWINDOW_H
#define GEOTIFFDEMVIEWERWINDOW_H

#include <iostream>
#include <QMainWindow>
#include "demcolormap.h"

class GeoTiffDEMViewerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GeoTiffDEMViewerWindow(QWidget *parent = nullptr);
    ~GeoTiffDEMViewerWindow();

    DEMColorMap *demCmap;

public slots:
    void dataCursorChanged(const QString &value);

protected:
    void closeEvent(QCloseEvent *event);

private:
};

#endif // GEOTIFFDEMVIEWERWINDOW_H
