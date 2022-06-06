#include "class_qwtplot.h"
#include <QFont>

class_QWTplot::class_QWTplot(){
    cnt_marker = 0;
}

class_QWTplot::~class_QWTplot( void ) {
    if ( p_grid != Q_NULLPTR ) {
        delete p_grid;
        p_grid = Q_NULLPTR;
    }
    if ( p_curve != Q_NULLPTR ) {
        delete p_curve;
        p_curve = Q_NULLPTR;
    }
    if ( p_plot != Q_NULLPTR ) {
        delete p_plot;
        p_plot = Q_NULLPTR;
    }
}

void class_QWTplot::prepare_plot(QwtPlot* ui_plot, QwtTextLabel * ui_qwt_lable){
    /*добавить возможность увеличивать и уменьшать график*/
    QFont font("Deja Vu", 15);

    p_plot = ui_plot;
    p_label = ui_qwt_lable;
    p_plot->setCanvasBackground(Qt::black);
    p_plot->setStyleSheet("background-color:black; color:gray; font: 12pt \"Deja Vu\";");

    QwtText xLable("F,МГц");
    QwtText yLable("Ampl");
    xLable.setFont(font);
    yLable.setFont(font);
    p_plot->setAxisTitle(QwtPlot::xBottom,xLable);
    p_plot->setAxisTitle(QwtPlot::yLeft,yLable);
    p_plot->setTitle("  ");

    //p_plot->setAxisScale(QwtPlot::yLeft,0,1);
   p_plot->axisAutoScale(QwtPlot::yLeft);
   p_plot->axisAutoScale(QwtPlot::xBottom);

    p_grid = new QwtPlotGrid;
    p_grid->setMajorPen(Qt::gray,1);//цвет линии сетки и толщина
    p_grid->attach(p_plot);//добавили сетку на график

}
void class_QWTplot::draw(QVector<double> &xData, QVector<double> &yData, QVector<double> &spline_xData)
{
    p_plot->detachItems(QwtPlotItem::Rtti_PlotItem,true);
    p_grid = new QwtPlotGrid;
    p_grid->setMajorPen(Qt::gray,1);//цвет линии сетки и толщина
    p_grid->attach(p_plot);//добавили сетку на график

    if(spline_xData.count() > 0){
        p_spline_curve = new QwtPlotCurve;
        p_spline_curve->setPen(Qt::green,1);
        p_spline_curve->setSamples(spline_xData,yData);
        p_spline_curve->attach(p_plot);
    }

    p_curve = new QwtPlotCurve;
    p_curve->setPen(Qt::yellow,1);
    p_curve->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // сглаживание
    p_curve->setSamples(xData,yData);
    p_curve->attach(p_plot);
    p_plot->replot();
}

void class_QWTplot::save(QString path)
{
    p_plot->grab().save(path,"JPG");
}


void class_QWTplot::add_marker(double *x, double *y)
{
    QwtText text;
    QwtText marker_text;
    text.setColor("#d53da8");
    text.setText(QString::number(++cnt_marker));
    QwtPlotMarker *p_marker = new QwtPlotMarker;
    QwtSymbol *p_symbol=new QwtSymbol(QwtSymbol::Triangle,QBrush(Qt::red),QPen(Qt::red),QSize(15,15));
    p_marker->setLabel(text);
    p_marker->setLabelAlignment( Qt::AlignTop) ;
    marker_list.append(p_marker);
    p_marker->setSymbol(p_symbol);
    p_marker->setValue(*x,*y);
    p_marker->attach(p_plot);  

    //marker_text.setColor("gray");
    //marker_text.setText( "Маркер №" + QString::number(cnt_marker) + "F = " + QString::number(*y) +"        " +"Ampl = "+ QString::number(*x));
   // p_label->setText(marker_text);
    p_plot->replot(); // ??

    //https://qwt.sourceforge.io/class_qwt_plot_marker.html
    //https://python.hotexamples.com/examples/qwt/QwtPlotMarker/setLineStyle/python-qwtplotmarker-setlinestyle-method-examples.html
}


void class_QWTplot::add_auto_marker(QVector<double> *markers)
{
    QwtText text;
    QwtText footer_text;
    QString pic_list;

    text.setColor("#d53da8");
    footer_text.setColor("gray");

    for(int i = 0; i < markers->length(); i += 2){
         text.setText(QString::number(i+1));
         QwtPlotMarker *p_marker = new QwtPlotMarker;
         QwtSymbol *p_symbol=new QwtSymbol(QwtSymbol::XCross,QBrush(Qt::red),QPen(Qt::red),QSize(15,15));
         p_marker->setLabel(text);
         p_marker->setLabelAlignment( Qt::AlignTop) ;
         p_marker->setSymbol(p_symbol);
         p_marker->setValue(markers->at(i),markers->at(i+1));
         p_marker->attach(p_plot);
         pic_list.append("Пик №" + QString::number(i+1) +  "F = " + QString::number(markers->at(i)) +"        " +"Ampl = "+ QString::number(markers->at(i+1)) + "\n");
   }
   footer_text.setText( pic_list);
   p_plot->setFooter(footer_text);
   p_plot->replot();
}

void class_QWTplot::change_xScale(double *start, double *end)
{
    p_plot->setAxisScale(QwtPlot::xBottom,*start,*end);
    p_plot->replot();
}

void class_QWTplot::delete_marker()
{

}



//https://habr.com/ru/post/212137/
// https://programtalk.com/python-examples/qwt.QwtPlotMarker/
//https://unixforum.org/viewtopic.php?t=136587
