#ifndef CLASS_QWTPLOT_H
#define CLASS_QWTPLOT_H

#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_marker.h>
#include <qwt_text.h>
#include <qwt_text_label.h>
#include <QPixmap>
#include <QString>

class class_QWTplot
{
public:
    class_QWTplot();
    ~class_QWTplot();

    void prepare_plot(QwtPlot* ui_plot, QwtTextLabel* ui_qwt_lable);
    void draw(QVector<double> &xData, QVector<double> &yData, QVector<double> &spline_xData);
    void add_marker(double *x, double *y);
    void add_auto_marker(QVector<double>* markers);
    void delete_marker();
    void save(QString path);
    void change_xScale(double *start, double *end);



    QwtPlot       *p_plot; // указатель на qwtPlot размещенный на форме UI
    QwtTextLabel  *p_label;
    QwtPlotGrid   *p_grid;
    QwtPlotCurve  *p_curve;
    QwtPlotCurve  *p_spline_curve;
    int           cnt_marker;
    QList<QwtPlotMarker*> marker_list;
    QList<QwtPlotMarker*> hand_marker_list;
    QList<QwtPlotMarker*> table_marker_list;

};

#endif // CLASS_QWTPLOT_H
