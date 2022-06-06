#include "marker_move.h"

marker_move::marker_move(QwtPlot *plot, QwtTextLabel *qwt_label): QObject( plot ),p_selectedMarker(NULL),p_selectedCurve(NULL)
{
    QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>( plot->canvas() );
    canvas->installEventFilter( this );
    //фокус, чтоб клавиатура работала
    canvas->setFocusPolicy( Qt::StrongFocus );
    canvas->setCursor( Qt::PointingHandCursor );
    canvas->setFocusIndicator( QwtPlotCanvas::ItemFocusIndicator );
    canvas->setFocus();
    p_label = qwt_label;
}

QwtPlot *marker_move::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}

const QwtPlot *marker_move::plot() const
{
    return qobject_cast<const QwtPlot *>( parent() );
}

bool marker_move::event( QEvent *ev )
{
    if ( ev->type() == QEvent::User )
        return true;
    return QObject::event( ev );
}

bool marker_move::eventFilter( QObject *object, QEvent *event )
{
    if ( plot() == NULL || object != plot()->canvas() )
        return false;

    switch( event->type() )
    {
    case QEvent::MouseButtonPress:
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
        select( mouseEvent->pos() );
        return true;
    }
    case QEvent::MouseMove:
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
        move( mouseEvent->pos() );
        return true;
    }
    case QEvent::KeyPress:
    {
        const QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
        switch( keyEvent->key() )
        {
            case Qt::Key_Left:
            {
                move_by(left_dir);
                return true;
            }
            case Qt::Key_Right:
            {
                move_by(right_dir);
                return true;
            }
            case Qt::Key_Delete:
            {
                delete_marker();
                return  true;
            }
        }
    }
    default:
        break;
    }
    return QObject::eventFilter( object, event );
}

void marker_move::select( const QPoint &pos )
{
    //выбирать ближайшую к кривой точку и на ней ставить маркер через сетвалью
    QwtText       text;
    double        dist;
    p_selectedCurve = NULL;
    p_selectedMarker = NULL;
    const QwtPlotItemList& itmList = plot()->itemList();
    for ( QwtPlotItemIterator it = itmList.begin();it != itmList.end(); ++it ){
        if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotMarker ){
            QwtPlotMarker *p_marker = static_cast<QwtPlotMarker *>( *it );
            double diff_x = plot()->invTransform(p_marker->xAxis(),pos.x()) - p_marker->xValue();
            double diff_y = plot()->invTransform(p_marker->yAxis(),pos.y()) - p_marker->yValue();
            if( diff_x < 0)
                diff_x *= -1;
            if( diff_y < 0)
                diff_y *= -1;
            if( diff_x < 0.5 && diff_y < 0.5 ){
                 p_selectedMarker = p_marker;
                 break;
            }
        }
        else if (( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve){
            QwtPlotCurve *p_curve = static_cast<QwtPlotCurve *>( *it );
            p_selectedCurve = p_curve;
        }
    }
    marker_ind  = p_selectedCurve->closestPoint(pos,&dist);
    current_marker_index = marker_ind;

    text.setColor("gray");
    text.setText( "Маркер №"+ p_selectedMarker->label().text() +"      F = " + QString::number(p_selectedMarker->xValue()) +"        " +"Ampl = "+QString::number(p_selectedMarker->yValue()));
    p_label->setText(text);

}

void marker_move::move_by(int direction)
{
    QwtPlotCanvas   *plotCanvas;
    QPointF         sample;
    QwtText         text;

    if ( !p_selectedMarker )
        return;

    if(direction == right_dir){
        sample = p_selectedCurve->sample(++current_marker_index);
    }
    else{
       if(current_marker_index !=  0)
           sample = p_selectedCurve->sample(--current_marker_index);
    }
    p_selectedMarker->setValue(sample.x(),sample.y());

    //добавили инфу о маркере в верхний лейбел
    text.setColor("gray");
    text.setText("Маркер №"+p_selectedMarker->label().text()+"      F = " + QString::number(sample.x()) +"       " +"Ampl = "+QString::number(sample.y()) );
    p_label->setText(text);

    plotCanvas =qobject_cast<QwtPlotCanvas *>( plot()->canvas() );
    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, true );
    plot()->replot();
    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, false );
}

void marker_move::move( const QPoint &pos )
{
    QwtPlotCanvas   *plotCanvas;
    QwtText         text;
    double          dist;

    if ( !p_selectedMarker )
        return;

    //маркер по линии едет
    int index = p_selectedCurve->closestPoint(pos,&dist);
    const QPointF sample = p_selectedCurve->sample(index);
    p_selectedMarker->setValue(sample.x(),sample.y());

    text.setColor("gray");
    text.setText("Маркер №"+p_selectedMarker->label().text()+"      F = " + QString::number(sample.x()) +"       " +"Ampl = "+QString::number(sample.y()) );
    p_label->setText(text);

    plotCanvas =qobject_cast<QwtPlotCanvas *>( plot()->canvas() );
    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, true );
    plot()->replot();
    plotCanvas->setPaintAttribute( QwtPlotCanvas::ImmediatePaint, false );

    //маркер полностью за мышкой в ручную
    /*x = plot()->invTransform(p_selectedMarker->xAxis(),pos.x());
    y = plot()->invTransform(p_selectedMarker->yAxis(),pos.y());*/
}

void marker_move::delete_marker()
{
    if(!p_selectedMarker)
        return;
    p_selectedMarker->detach();
    plot()->replot();
}
