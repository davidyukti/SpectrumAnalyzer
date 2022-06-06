#include "classgenerator.h"

classGenerator::classGenerator(){
    m_isRunning = false;
}

void classGenerator::process( ){
    qDebug() << "start TX_stream" << endl;

    double F = 5e6;
    double sampleRate = 15e6;
    const double f_ratio = F/sampleRate;
    int buffer_size = 1024;

    double tx_buffer[2*buffer_size];
    for (int i = 0; i <buffer_size; i++) {
            const double pi = acos(-1);
            double w = 2 * pi * i * f_ratio;
            tx_buffer[2*i] = cos(w);
            tx_buffer[2*i+1] = sin(w);
    }
    const int send_cnt = int(buffer_size*f_ratio) / f_ratio;

    m_isRunning = true;
    while(m_isRunning == true){
        //qDebug() << "m_isRunning:" << m_isRunning;
        //lms_write_TX();
        int ret = LMS_SendStream(&tx_stream, tx_buffer,  send_cnt, nullptr, 1000);
        //qDebug() << "send" << endl;
    }
    lms_stop_TX();
    emit finished();
}



