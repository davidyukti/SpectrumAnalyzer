#include "mainwindow.h"

double buff[2048];

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

//в футер значения пиков(если не совпадает с таблицей частот выделить другим цветом)
// в верхний лейбел значения маркеров  + измерения
