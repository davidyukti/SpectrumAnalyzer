#ifndef LMS_H
#define LMS_H

#include <QDebug>
#include <iostream>
#include "lime/LimeSuite.h"

#define MAX_DEVICE 8

using namespace std;

extern lms_stream_t rx_stream;
extern lms_stream_t tx_stream;
extern bool         dev_isReady;
extern bool         dev_RXstream;
extern bool         dev_TXstream;


extern double buff[2048];
//extern double tx_buffer[1024*2];


int     lms_open(void);
int     lms_init(void);
void    lms_close(void);
int     lms_error(void);
int     lms_config_TXstream(const double *p_frequency, const double *p_sample_rate, const unsigned int *p_channel);
int     lms_config_RXstream(const double *p_frequency, const double *p_sample_rate, const unsigned int *p_channel);
int     lms_config_RXTX_stream(const double *p_frequency, const double *p_sample_rate, const unsigned int *p_channel_in, const unsigned int *p_channel_out);
int     lms_stop_RX(void);
int     lms_stop_TX(void);

int     lms_read_RX();




//void lms_write_TX(double buff, int send_size);
void lms_write_TX();





#endif // LMS_H
