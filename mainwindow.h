#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QApplication>
#include <QThread>
#include <QMessageBox>
#include <QTimer>
#include <QFileDialog>
#include <fftw3.h>
#include "LMS.h"
#include "class_qwtplot.h"
#include "analyzer.h"
#include "classgenerator.h"
#include "marker_move.h"
#include "marker.h"
#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "interpolation.h"

using namespace alglib;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
;

private slots:
    // Настройка SDR
    void on_btn_connect_dev_clicked(bool checked);    // Подключение SDR
    void on_btn_start_generate_clicked(bool checked); // Передача сигнала
    void on_btn_start_RX_clicked(bool checked);       // Прием сигнала
    void on_btn_test_sig_clicked(bool checked);       // Кольцо с тестовым синалом

    // Таблица частот
    void on_btn_add_freq_clicked();
    void on_btn_load_freqTable_clicked();
    void on_btn_clear_freqTable_clicked();
    void on_btn_save_freqTable_clicked();

    void on_checkBox_table_clicked(bool checked);    // Выбрать постановку маркеров по таблице частот
    void on_checkBox_by_hand__clicked(bool checked); // Выбрать постановку маркеров вручную

    void on_btn_addMarker_clicked(); // кнопка для добавления маркера вручную

    void on_btn_AM_clicked();       // Расчет глубины АМ сигнала
    void on_btn_FM_dev_clicked();   // Расчет девиации ЧМ сигнала
    void on_btn_width_clicked();    // Расчет ширины полосы

    void on_btn_change_xAxis_clicked(); // Поменять полосу обзора

    void make_plot(); // Построение спектра

    void on_btn_save_pic_clicked(); // Сохранить график

private:
    Ui::MainWindow *ui;
    class_QWTplot   plot;
    analyzer        *p_lms_analyzer;
    classGenerator  *p_lms_generator;
    QThread         *p_RX_thread;
    QThread         *p_TX_thread;
    QTimer          *p_tmr_draw;
    QMutex          mtx_read_RX;
    QVector<double> xData;
    QVector<double> yData;
    QVector<double> f_marker;
    QStringList     headers;
    QVector<double> freq_table_list;
    QVector<double> pic_list;
    QVector<double> spline_dataX;
    QVector<double> spline_dataY;

    void compair_pic_table();
    void auto_pick();
    void spline_data();

signals:
    void finished();
    void ready();
};
#endif // MAINWINDOW_H
