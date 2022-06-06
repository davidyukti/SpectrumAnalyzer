#ifndef MARKER_MOVE_H
#define MARKER_MOVE_H

#include <QObject>
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_picker.h>
#include <qwt_text_label.h>
#include <QEvent>
#include <QApplication>
#include <QMouseEvent>
#include <math.h>

class marker_move: public QObject
{   Q_OBJECT
public:
    marker_move(QwtPlot *plot, QwtTextLabel *qwt_label);
    virtual bool eventFilter( QObject *, QEvent * );
    virtual bool event( QEvent * );

private:
    void    select( const QPoint &pos ); //выбор маркера
    void    move(const QPoint &pos);    //передвижение мышью вручную
    void    move_by(int direction);     //передвижение cстрелками по линии
    void    delete_marker();

    QwtPlot         *plot();
    const QwtPlot   *plot() const;
    QwtTextLabel    *p_label;
    QwtPlotMarker   *p_selectedMarker;
    QwtPlotCurve    *p_selectedCurve;
    int             marker_ind; //соответствие маркера точки на кривой (ее индекс)
    int             current_marker_index;

    enum direction{
        left_dir,
        right_dir,
    };

};

#endif // MARKER_MOVE_H
