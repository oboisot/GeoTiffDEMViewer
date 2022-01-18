#ifndef QDEMCOLORMAP_H
#define QDEMCOLORMAP_H

#include <iostream>
#include <cmath>
#include <filesystem>
#include <QtConcurrent>
#include <QGuiApplication>
#include "qcustomplot.h"
#include "src/qgeotiffdem.h"

namespace fs = std::filesystem;

enum QDEMStatusColor
{
    /*! Normal status.*/
    Normal,
    /*! Warning status.*/
    Warning,
    /*! Error status.*/
    Error
};

class QDEMColorMap : public QCustomPlot
{
    Q_OBJECT

public:
    explicit QDEMColorMap();
    ~QDEMColorMap();

    // Getter
    QCPColorMap *cmap();
    //
    void openDEM(const fs::path &demPath);
    void plotDEM(bool axesEquals);
    void setBackgroundColor(const QColor &color);
    void setAxisRectBackgroundColor(const QColor &color);
    void setAxesColor(const QColor &color);
    // Interaction functions
    void resetZoom();
    /*!
     * \brief zoomIn
     * \param [in] zoomStep
     * \param [in] tX The X translation (without scale factor) when used with
     *                a mouse click, to zoom an keep this point at the same
     *                place on screen
     * \param [in] tY The Y translation (without scale factor) when used with
     *                a mouse click, to zoom an keep this point at the same
     *                place on screen
     */
    void zoomIn(const int &zoomStep=1, const double &tX=0.0, const double &tY=0.0);
    void zoomOut(const int &zoomStep=1, const double &tX=0.0, const double &tY=0.0);


signals:
    void statusChanged(const QString &status, const QDEMStatusColor &color, const int &msec=0);
    void zoomChanged(const int &zoomLevel);
    void cmapCursorPosChanged(const QString &value);
    void progressChanged(const double &progress);
    void cursorChanged(const QCursor &cursor);

private slots:
    void onProgressChanged(const double &progress);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private:
    // Class initialization parameters
    QGeoTiffDEM *m_dem;
    QCPColorGradient m_gradient;
    GeoTiffDEMinterp m_interp;
    int m_zoomLevel, m_zoomLevelMax, m_zoomInterpThreshold;
    double m_zoomFactor, m_betaIn, m_betaOut;
    bool m_isPlotting, m_isMousePressedInCmapBBox, m_selectRectEnabled;
    //
    QCPColorMap *m_cmap;
    QCPColorScale *m_cscale;
    GeoTiffDEMAxesUnit m_axes;
    std::size_t m_bufXsize, m_bufYsize;
    double m_Xmin, m_Ymin, m_Xmax, m_Ymax,
           m_X0, m_Y0, m_X1, m_Y1,
           m_noDataValue,
           m_zbufZmin, m_zbufZmax,
           m_progress_old, m_progress;
    QPoint m_mousePressPos;
    // Private methods
    void initBufSizeFromScreenSize();
    void replotDEM(bool axesEquals);
    void getCmapBBox(double &X0, double &Y1, double &X1, double &Y0);
    bool needPlotFromBBox();
    bool isMouseEventInCMapBBox(QPointF position);
    bool isMouseEventInCScaleRect(QPointF position);
    void computeDEMZoomFactors();
    /*!
     * \brief computeZoomInFactor
     *
     * When zooming in, new bounding box position is computed
     * as function of the zoomStep. In case of a unit zoomStep
     * \f$n=1\f$ the new \f$X_0\f$ is computed as:
     *
     * \f[
     *     X_0(1)=X_0(0)+\beta\left(X_1(0)-X_0(0)\right)
     * \f]
     *
     * with factor \f$\beta\f$ computed from the zoom factor \f$\alpha\geq1\f$:
     *
     * \f[
     *     \beta=\dfrac{1}{2}\left(1-\dfrac{1}{\alpha}\right)
     * \f]
     *
     * Generally, for a zoomStep: \f$n>1\f$, coming from a previous
     * zoom level \f$p<n\f$ the relationship for the new minimum
     * \f$X_0\f$ bound is given by the relationship:
     *
     * \f[
     *     X_{0,n}=X_{0,p}+v(n-p)\beta\left(X_{1,p}-X_{0,p}\right)
     * \f]
     *
     * where \f$v(n)\f$ is a serie given by:
     *
     * \f[
     *     v(n)=1+v(n-1)(1-2\beta)\quad\quad\mathrm{with: }v(0)=0
     * \f]
     *
     * The factor \f$v(n-p)\beta\f$ is thus the same for each point
     * of the new bounding box.
     *
     * This function computes the aformentioned factor.
     *
     * \param zoomStep
     * \return
     */
    void computeZoomInFactors(const int &zoomStep, double &zf, double &tf);
    void computeZoomOutFactors(const int &zoomStep, double &zf, double &tf);
    //
};

#endif // QDEMCOLORMAP_H
