#ifndef CLASSGENERATOR_H
#define CLASSGENERATOR_H

#include <QObject>
#include <QDebug>
#include <math.h>
#include "LMS.h"

class classGenerator: public QObject
{
     Q_OBJECT
public:
    classGenerator();
    bool            m_isRunning;
    unsigned int    channel_out;
    double          ampl;
    double          freq_tone;//несущая частота
    double          frequency;//центральная
    double          sampleRate;//дискретизации

public slots:
    void process();
signals:
    void finished();

private:

};

#endif // CLASSGENERATOR_H
