#include "analyzer.h"
#include <QDebug>

analyzer::analyzer(QMutex *p_mutex){
    m_mtx_read_RX = p_mutex;
    m_isRunning = false;
}

void analyzer::process(){
    int samples_recv;
    qDebug() << "Start read RX" << endl;
    m_isRunning = true;
    while(m_isRunning == true){
        m_mtx_read_RX->lock();
        samples_recv = lms_read_RX();
        m_mtx_read_RX->unlock();
    }
    lms_stop_RX();
    emit finished();
}


