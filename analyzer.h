#ifndef ANALYZER_H
#define ANALYZER_H

#include <QObject>
#include <QMutex>
#include <stdbool.h>
#include "LMS.h"



class analyzer: public QObject
{
    Q_OBJECT
public:
    analyzer(QMutex *p_mutex);
    bool            m_isRunning;
    unsigned int    fft_size;
    double          frequency;
    double          sampleRate;

public slots:
    void process();

signals:
    void finished();
private:
    QMutex  *m_mtx_read_RX;
};

#endif // ANALYZER_H
