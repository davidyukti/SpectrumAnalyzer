#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    plot.prepare_plot(ui->qwtPlot, ui->lbl_marker_info);
    (void) new marker_move(ui->qwtPlot,ui->lbl_marker_info);

    headers << "Частота, МГц" << "Амплитуда, ";
    ui->tableWidget_freqTable->setColumnCount(2);
    ui->tableWidget_freqTable->setRowCount(4);
    ui->tableWidget_freqTable->setShowGrid(true);
    ui->tableWidget_freqTable->setHorizontalHeaderLabels(headers);
    ui->tableWidget_freqTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
    ui->btn_addMarker->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_connect_dev_clicked(bool checked)
{   QMessageBox msgBox;
    int ret;
    if( checked == true){
        ret = lms_open();
        if(ret != EXIT_SUCCESS){
            QMessageBox::critical(this,"Ошибка","Невозможно открыть устройство");
            ui->btn_connect_dev->setChecked(false);
            return;
        }
        ret = lms_init();
        if(ret != EXIT_SUCCESS){
            QMessageBox::critical(this,"Ошибка","Ошибка инициализации устройства");
            ui->btn_connect_dev->setChecked(false);
            return;
        }
        ui->label_connect_dev->setStyleSheet("QLable{color:green}");
        ui->label_connect_dev->setText("Устройство подключено");
    }
    else {
        lms_close();
        ui->label_connect_dev->setStyleSheet("QLable{color:red}");
        ui->label_connect_dev->setText("Устройство отключено");
    }
}

void MainWindow::make_plot(){
    fftw_complex*       samples_in;
    fftw_complex*       samples_out;

    xData.clear();
    yData.clear();

    samples_in = static_cast<fftw_complex*>(fftw_malloc(p_lms_analyzer->fft_size*sizeof(fftw_complex)));
    samples_out = static_cast<fftw_complex*>(fftw_malloc(p_lms_analyzer->fft_size*sizeof(fftw_complex)));

    //забрать данные из буфера в который пишутся пришедшие данные
     mtx_read_RX.lock();
     for (unsigned int j = 0; j < p_lms_analyzer->fft_size; ++j){
            samples_in[j][0] = buff[2 * j]; //Re
            samples_in[j][1] = buff[2*j+1]; // Im
     }
     mtx_read_RX.unlock();

     fftw_plan plan = fftw_plan_dft_1d(p_lms_analyzer->fft_size,(fftw_complex*)&samples_in[0],(fftw_complex*)&samples_out[0],FFTW_FORWARD,FFTW_ESTIMATE);
     fftw_execute(plan);

     for(unsigned int j = 0; j < p_lms_analyzer->fft_size/2; j++){
             double ampl = sqrt(samples_out[j][0]*samples_out[j][0]+samples_out[j][1]*samples_out[j][1]);
             double f = j*p_lms_analyzer->sampleRate/p_lms_analyzer->fft_size;
             xData.append(f/1e6);
             yData.append(20* log(ampl));
     }
     fftw_destroy_plan(plan);

     spline_dataX.clear();
     if(ui->checkBox_auto->isChecked()){
         pic_list.clear();
         auto_pick();
         compair_pic_table();
         plot.add_auto_marker(&pic_list);
      }

      plot.draw(xData,yData,spline_dataX);
}

void MainWindow::auto_pick()
{
    //апроксимировать данные и по ним искать максимум
//Методы сглаживания есть. Можно moving average, гауссовкий фильтр, полиномами, wavelets, сплайнами
    //поиск пиков
    //Самый простой пример сглаживания - скользящая средняя. Затем вы можете найти пики на этой скользящей средней. А затем вы просто возвращаетесь к своим исходным данным и берете ближайший пик к тому, который вы нашли на скользящей средней.

    // https://www.alglib.net/interpolation/leastsquares.php !!!

    // Penalized regression spline
    spline_data();
    //берем точку и сравниваем с ней ее соседей д.б. с одной стороны увеличение с другой спад
    for(int i = 1; i < spline_dataY.count()-1; i++){
        if((yData.at(i-1) < yData.at(i) && yData.at(i) > yData.at(i+1))){
            pic_list.append(xData.at(i));
            //pic_list.append(spline_dataY.at(i));
            pic_list.append(yData.at(i));
        }
    }
    //выбрать точкую близкую к реальным данным

    /*
     * старый вариант
    double          average;
    double          sum = 0;
      for(int i = 0; i < yData.count(); i++)
        sum += yData.at(i);
    average = sum / yData.count() ; //среднее чтобы шум отбросить
    qDebug() << average;

    plot.cnt_marker = 0;


    for(int i = 1; i < yData.count() - 1; i++){
        if((yData.at(i-1) < yData.at(i) && yData.at(i) > yData.at(i+1)) && yData.at(i) > average + 40 ){
            pic_list.append(xData.at(i));
            pic_list.append(yData.at(i));
        }
    } */
}

void MainWindow:: spline_data()
{
    real_1d_array       x;
    real_1d_array       y;
    ae_int_t            info;
    spline1dinterpolant s;
    spline1dfitreport   rep;
    double              rho = 3.0; // чем меньше сглаживание, тем ближе к реальной функции М = 50 количество узлов сплайна

    x.setlength(yData.length());
    y.setlength(xData.length());

    for(int i = 0; i < xData.length(); i++){
        x[i] = xData.at(i);
        y[i] = yData.at(i);
    }

    spline1dfitpenalized(x, y, 50, rho, info, s, rep);

    for(int i = 0; i < yData.length(); i++)
        spline_dataY.append( spline1dcalc(s, yData.at(i)));
}


void MainWindow::on_btn_start_RX_clicked(bool checked)
{
    unsigned int channel_in;

    if( checked == true){
        if(dev_isReady == false){
            QMessageBox::critical(this,"Ошибка","Подлючите устройство");
            ui->btn_start_RX->setChecked(false);
            return;
        }
        ui->checkBox_by_hand_->setEnabled(false);
        ui->checkBox_table->setEnabled(false);
        ui->btn_AM->setEnabled(false);
        ui->btn_FM_dev->setEnabled(false);


        p_lms_analyzer = new analyzer(&mtx_read_RX);
        p_RX_thread = new QThread;

        channel_in = ui->comboBox_channel_in->currentText().toUInt();
        p_lms_analyzer->fft_size = ui->comboBox_fft_size->currentText().toUInt();
        p_lms_analyzer->frequency = ui->doubleSpinBox_Fcentral->value() * 1e6; //МГц
        p_lms_analyzer->sampleRate = ui->doubleSpinBox_sampleRate->value() * 1e6;

        p_tmr_draw = new QTimer(this);
        connect(p_tmr_draw,SIGNAL(timeout()),this,SLOT(make_plot()));
        p_tmr_draw->setInterval( 1000 );

        lms_config_RXstream(&(p_lms_analyzer->frequency),&(p_lms_analyzer->sampleRate),&channel_in);
        p_lms_analyzer->moveToThread(p_RX_thread);
        connect(p_RX_thread,SIGNAL( started() ),p_lms_analyzer,SLOT( process() ) );
        connect(p_lms_analyzer,SIGNAL( finished() ),p_RX_thread,SLOT( quit() ) );
        connect(p_lms_analyzer,SIGNAL( finished() ), p_lms_analyzer, SLOT( deleteLater() ) );
        connect(p_RX_thread,SIGNAL( finished() ), p_RX_thread, SLOT( deleteLater() ) );
        p_RX_thread->start();
        p_tmr_draw->start();
    }
    else{
        p_tmr_draw->stop();
        p_lms_analyzer->m_isRunning = false;

        ui->checkBox_by_hand_->setEnabled(true);
        ui->checkBox_table->setEnabled(true);
        ui->btn_AM->setEnabled(true);
        ui->btn_FM_dev->setEnabled(true);
        //plot.cnt_marker = 0;
    }
}

void MainWindow::on_btn_start_generate_clicked(bool checked)
{
    unsigned int channel_out;
    if(checked == true){
        if(dev_isReady == false){
            QMessageBox::critical(this,"Ошибка","Подлючите устройство");
            ui->btn_start_generate->setChecked(false);
            return;
        }
        p_TX_thread = new QThread;
        p_lms_generator = new classGenerator();

        channel_out = ui->comboBox_channel_out->currentText().toUInt();
        p_lms_generator->ampl = ui->doubleSpinBox_Ampl->value();
        p_lms_generator->freq_tone = ui->doubleSpinBox_Ftone->value()*1e6;
        p_lms_generator->frequency = ui->doubleSpinBox_Fcentral->value() * 1e6; //МГц
        p_lms_generator->sampleRate = ui->doubleSpinBox_sampleRate->value() * 1e6;

        p_lms_generator->moveToThread(p_TX_thread);
        connect(p_TX_thread,SIGNAL( started() ),p_lms_generator,SLOT( process() ) );
        connect(p_lms_generator,SIGNAL( finished() ),p_TX_thread,SLOT( quit() ) );
        connect(p_lms_generator,SIGNAL( finished() ), p_lms_generator, SLOT( deleteLater() ) );
        connect(p_TX_thread,SIGNAL( finished() ), p_TX_thread, SLOT( deleteLater() ) );
        lms_config_TXstream(&(p_lms_generator->frequency),&(p_lms_generator->sampleRate),&channel_out);
         p_TX_thread->start();
    }
    else{
        p_lms_generator->m_isRunning = false;
    }
}


//что не работает
void MainWindow::on_btn_test_sig_clicked(bool checked)
{
    unsigned int    channel_in;
    unsigned int    channel_out;
   // unsigned int    fft_size;
   // double          ampl;
   // double          freq_tone;//несущая частота
   double          frequency;//центральная
   double          sampleRate;//дискретизации

    if(checked == true){
        if(dev_isReady == false){
            QMessageBox::critical(this,"Ошибка","Подлючите устройство");
            ui->btn_start_generate->setChecked(false);
            return;
        }
        //задать полосу обзора
        //plot.change_xScale(&f_start,&f_end);
        p_lms_analyzer = new analyzer(&mtx_read_RX);
        p_lms_generator = new classGenerator();

        p_tmr_draw = new QTimer(this);
        connect(p_tmr_draw,SIGNAL(timeout()),this,SLOT(make_plot()));
        p_tmr_draw->setInterval( 1000 );

        channel_in = ui->comboBox_channel_in->currentText().toUInt();
        channel_out = ui->comboBox_channel_out->currentText().toUInt();
        p_lms_generator->freq_tone = ui->doubleSpinBox_Ftone->value()*1e6;
        p_lms_generator->ampl = ui->doubleSpinBox_Ampl->value();
        p_lms_generator->frequency = ui->doubleSpinBox_Fcentral->value() * 1e6; //МГц
        p_lms_generator->sampleRate = ui->doubleSpinBox_sampleRate->value() * 1e6;
        p_lms_analyzer->fft_size = ui->comboBox_fft_size->currentText().toUInt();
        p_lms_analyzer->frequency = ui->doubleSpinBox_Fcentral->value() * 1e6; //МГц
        p_lms_analyzer->sampleRate = ui->doubleSpinBox_sampleRate->value() * 1e6;

        p_TX_thread = new QThread;
        p_lms_generator->moveToThread(p_TX_thread);
        connect(p_TX_thread,SIGNAL( started() ),p_lms_generator,SLOT( process() ) );
        connect(p_lms_generator,SIGNAL( finished() ),p_TX_thread,SLOT( quit() ) );
        connect(p_lms_generator,SIGNAL( finished() ), p_lms_generator, SLOT( deleteLater() ) );
        connect(p_TX_thread,SIGNAL( finished() ), p_TX_thread, SLOT( deleteLater() ) );

        p_RX_thread = new QThread;
        p_lms_analyzer->moveToThread(p_RX_thread);
        connect(p_RX_thread,SIGNAL( started() ),p_lms_analyzer,SLOT( process() ) );
        connect(p_lms_analyzer,SIGNAL( finished() ),p_RX_thread,SLOT( quit() ) );
        connect(p_lms_analyzer,SIGNAL( finished() ), p_lms_analyzer, SLOT( deleteLater() ) );
        connect(p_RX_thread,SIGNAL( finished() ), p_RX_thread, SLOT( deleteLater() ) );

        lms_config_RXTX_stream(&p_lms_analyzer->frequency,&p_lms_generator->sampleRate,&channel_in,&channel_out);
       // lms_config_RXstream(&(p_lms_analyzer->frequency),&(p_lms_analyzer->sampleRate),&channel_in);
        //lms_config_TXstream(&(p_lms_generator->frequency),&(p_lms_generator->sampleRate),&channel_out);

        p_RX_thread->start();
        p_TX_thread->start();
        p_tmr_draw->start();
        qDebug() << "Старт";
    }
    else{
        p_tmr_draw->stop();
        p_lms_generator->m_isRunning = false;
        p_lms_analyzer->m_isRunning = false;
        plot.cnt_marker = 0;
        qDebug() << "Стоп";
    }
}

//сохранить изображение с графика
void MainWindow::on_btn_save_pic_clicked()
{
    QString path = QFileDialog::getSaveFileName(this,"Сохранить файл","","*.jpg");
    plot.save(path);
}


//-------МАРКЕРЫ-----------------(выставляются после завершения работы приемника)

// Выставлять по таблице частот

void MainWindow::on_checkBox_table_clicked(bool checked)
{
    if(checked == false)
        return;
    if(freq_table_list.count() == 0){
        QMessageBox::critical(this,"Ошибка","Введите таблицу частот");
        return;
    }
    for(int i = 0 ; i <freq_table_list.count(); i += 2){
        double x = freq_table_list.at(i);
        double y = freq_table_list.at(i+1);
        plot.add_marker(&x,&y);
    }
}
//


//    Вручную ставим маркеры   //
void MainWindow::on_checkBox_by_hand__clicked(bool checked)
{
    if( checked == true)
        ui->btn_addMarker->setEnabled(true);
    else
        ui->btn_addMarker->setEnabled(false); 
}

void MainWindow::on_btn_addMarker_clicked()
{
    //f_marker.clear();
    //запихнуть в произвольную точку
    double x = 0.1;
    double y = 0.1;
    plot.add_marker(&x,&y);
   // f_marker.append(x);
   // f_marker.append(y);
   //compair_pic_table();
}

//удаление??

//

void MainWindow::on_btn_change_xAxis_clicked()
{
    double f_start = ui->ledit_start_F->text().toLong();
    double f_end = ui->ledit_end_F->text().toLong();
    plot.change_xScale(&f_start,&f_end);
}

//сравнение пиков с таблицей частот
void MainWindow::compair_pic_table()
{
    //погрешность допустимая?????????????????
    bool find_pic = false;
    if( pic_list.count() == 0){
        return;
    }
    for(int i = 0 ; i < pic_list.count(); i += 2){
        double pic_F = pic_list.at(i);
        double pic_A = pic_list.at(i+1);
        for(int j = 0; j < freq_table_list.count(); j += 2){
            if( pic_F == freq_table_list.at(j) && pic_A == freq_table_list.at(j+1)){
                find_pic = true;
                freq_table_list.remove(j); // уменьшить список чтоб меньше по нему ходить
                freq_table_list.remove(j+1);
                qDebug() << "Пик " << pic_F << "," << pic_A << " найден";
                break;
            }
            else{
                find_pic = false;
            }
        }
        if(find_pic == false){
            qDebug() << "Пик " << pic_F << "," << pic_A << " не найден";
        }
    }
}


//---------ТАБЛИЦА ЧАСТОТ---------------------

void MainWindow::on_btn_add_freq_clicked()
{
    //ui->tableWidget_freqTable->insertRow(row++);
    ui->tableWidget_freqTable->insertRow(ui->tableWidget_freqTable->rowCount());
}

//
void MainWindow::on_btn_load_freqTable_clicked()
{
    QString file_path = QFileDialog::getOpenFileName(this,"Открыть файл","","*.txt");
    QFile freq_file(file_path);
    if(!freq_file.open(QIODevice::ReadOnly)){
       QMessageBox::critical(this,"Ошибка","Ошибка открытия файла " + file_path);
       return;
    }
    while(!freq_file.atEnd()){
        QString line = freq_file.readLine();

    }
    freq_file.close();
}

void MainWindow::on_btn_clear_freqTable_clicked()
{
    for(int i = ui->tableWidget_freqTable->rowCount() - 1; i >= 0; i--)
        ui->tableWidget_freqTable->removeRow(i);
    freq_table_list.clear();
}

void MainWindow::on_btn_save_freqTable_clicked()
{
    for(int i = 0; i < ui->tableWidget_freqTable->rowCount(); i++){
       freq_table_list.append(ui->tableWidget_freqTable->item(i,0)->text().toDouble()); // f
       freq_table_list.append(ui->tableWidget_freqTable->item(i,1)->text().toDouble()); // ampl
    }
}
//

// ИЗМЕРЕНИЯ

void MainWindow::on_btn_AM_clicked()
{
    QVector<double> pic;
    double          A0;
    double          A1;

    if(pic_list.count() == 0){
        return;
    }
    for( int i = 0; i < pic.count(); i += 2){
        if(pic.at(i) == ui->doubleSpinBox_Fcentral->value() * 1e6){
           A0 = pic.at(i+1);
           A1 = pic.at(i+3);
        }
    }
    // проверить если пустые знаения
    double  am = 2 * pow(10,(A0 - A1)/20);
    qDebug() << "am = " << am;
    ui->lbl_info->setText("Глубина АМ сигнала = " + QString::number(am) + "%");

    // найти несущую, на ней первая палка A0
    // найти следующий пик справа или слева Aбок
    // А = A0 - Aбок
    // ам % = 2 * pow(10, A/20)
}

void MainWindow::on_btn_FM_dev_clicked()
{
    //по ширине спектра ЧМ сигнала
    //найти пики слева и справа от несущей
    // ( x1 - x2 ) / 2 частота девиации
}

void MainWindow::on_btn_width_clicked()
{
    // найти максимум и от него отложить уровень накотором измерять ширину
}
