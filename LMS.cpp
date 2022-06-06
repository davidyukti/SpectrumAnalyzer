#include "LMS.h"

lms_device_t* p_device = NULL; //Device structure, should be initialize to NULL
lms_stream_t rx_stream;
lms_stream_t tx_stream;
bool         dev_isReady;
bool         dev_RXstream;
bool         dev_TXstream;


int lms_error(){
    cout << "LMS error" << LMS_GetLastErrorMessage();
    dev_isReady = false;
    if (p_device != NULL)
        LMS_Close(p_device);
    return( EXIT_FAILURE );
}

int lms_open(){
    int n;
    dev_isReady = false;
    lms_info_str_t list[MAX_DEVICE];
    if((n = LMS_GetDeviceList(list)) < 0){ //NULL can be passed to only get number of devices
         lms_error();
         return ( EXIT_FAILURE );
    }
    cout << "Devices found: " << n << endl;
    for(int i = 0; i < n; i++)//print device list
         cout << i << ": " << list[i] << endl;
    if(LMS_Open(&p_device, list[0], NULL)){  //open the first device
        lms_error();
        return ( EXIT_FAILURE );
    }
    dev_isReady = true;
    return ( EXIT_SUCCESS );
}

int lms_init(){
    if(LMS_Init(p_device) != 0){
        lms_error();
    }
    dev_isReady = true;
    return ( EXIT_SUCCESS );
}
void lms_close(){
    LMS_Close(p_device);
    dev_isReady = false;
}

int lms_config_RXstream(const double *p_frequency, const double *p_sample_rate, const unsigned int *p_channel){
    dev_RXstream = false;
    //Enable RX channel
    if(LMS_EnableChannel(p_device, LMS_CH_RX, *p_channel, true) != 0)
        lms_error();
    //Set center frequency
    if (LMS_SetLOFrequency(p_device, LMS_CH_RX, *p_channel,*p_frequency) != 0)
         lms_error();
   //Set sample rate
   if (LMS_SetSampleRate(p_device, *p_sample_rate, 2) != 0)
         lms_error();
   //выюрали антенну
    if(LMS_SetAntenna(p_device,LMS_CH_RX,*p_channel,LMS_PATH_LNAW) !=0)
       lms_error();

    rx_stream.channel = *p_channel; //channel number
    rx_stream.fifoSize = 1024 * 1024; //fifo size in samples
    //rx_stream.throughputVsLatency = 1.0;
    rx_stream.throughputVsLatency = 0.5;
    rx_stream.isTx = false; //RX channel
    rx_stream.dataFmt = lms_stream_t::LMS_FMT_F32; //32-bit floats
    if (LMS_SetupStream(p_device, &rx_stream) != 0)
        lms_error();
    if(LMS_StartStream(&rx_stream) != 0)
        lms_error();
    dev_RXstream = true;
    return ( EXIT_SUCCESS );
}

int lms_config_TXstream(const double *p_frequency, const double *p_sample_rate, const unsigned int *p_channel){
    dev_TXstream = true;
    //Enable TX channel,Channels are numbered starting at 0
    if (LMS_EnableChannel(p_device, LMS_CH_TX, *p_channel, true)!=0)
        lms_error();
    //Set center frequency
    if (LMS_SetLOFrequency(p_device,LMS_CH_TX, *p_channel, *p_frequency)!=0)
        lms_error();
    if (LMS_SetSampleRate(p_device, *p_sample_rate, 2) != 0)
          lms_error();
    //select TX1_1 antenna
    if (LMS_SetAntenna(p_device, LMS_CH_TX, *p_channel, LMS_PATH_TX1)!=0)
        lms_error();
    //set TX gain
    if (LMS_SetNormalizedGain(p_device, LMS_CH_TX, *p_channel, 0.9) != 0)
        lms_error();
    //calibrate Tx, continue on failure
    LMS_Calibrate(p_device, LMS_CH_TX, *p_channel, *p_sample_rate, 0);

    tx_stream.channel = *p_channel;                  //channel number
    //tx_stream.fifoSize = 256*1024;          //fifo size in samples
    tx_stream.fifoSize = 1024 * 1024;
    //tx_stream.throughputVsLatency = 0.5;    //0 min latency, 1 max throughput
    tx_stream.throughputVsLatency = 1.0;
    tx_stream.dataFmt = lms_stream_t::LMS_FMT_F32; //floating point samples
    tx_stream.isTx = true;                  //TX channel
    if (LMS_SetupStream(p_device, &tx_stream) != 0)
        lms_error();
    if(LMS_StartStream(&tx_stream) != 0)
        lms_error();
    dev_TXstream = true;
    return ( EXIT_SUCCESS );
}

int lms_stop_RX(void){
    if(LMS_StopStream(&rx_stream) != 0)
        lms_error();
    if(LMS_DestroyStream(p_device, &rx_stream) != 0)
        lms_error();
    dev_RXstream = false;
    qDebug() << "lms_stop_RX";
    return ( EXIT_SUCCESS );
}

int lms_stop_TX(void){
    if(LMS_StopStream(&tx_stream) != 0)
        lms_error();
    if(LMS_DestroyStream(p_device, &tx_stream)  != 0)
        lms_error();
    dev_TXstream = false;
    qDebug() << "lms_stop_TX";
    return ( EXIT_SUCCESS );
}

int lms_read_RX(){
    int samplesRead = LMS_RecvStream(&rx_stream, buff, 2048-1, NULL, 1000);
    return samplesRead;
}


//что то не то
int lms_config_RXTX_stream(const double *p_frequency, const double *p_sample_rate, const unsigned int *p_channel_in,const unsigned int *p_channel_out){

    //Enable RX channel,Channels are numbered starting at 0
    if (LMS_EnableChannel(p_device, LMS_CH_TX, *p_channel_in, true)!=0)
        lms_error();
    //Enable TX channel
    if(LMS_EnableChannel(p_device, LMS_CH_RX, *p_channel_out, true) != 0)
        lms_error();

    if (LMS_SetLOFrequency(p_device, LMS_CH_RX, *p_channel_in, *p_frequency) != 0)
        lms_error();
    if (LMS_SetLOFrequency(p_device,LMS_CH_TX, *p_channel_out, *p_frequency)!=0)
        lms_error();
    if (LMS_SetSampleRate(p_device, *p_sample_rate, 2) != 0)
          lms_error();

    //select TX1_1 antenna
    if (LMS_SetAntenna(p_device, LMS_CH_TX, *p_channel_out, LMS_PATH_TX1)!=0)
        lms_error();
    //выбрать антенну
    if(LMS_SetAntenna(p_device,LMS_CH_RX,*p_channel_in,LMS_PATH_LNAW) !=0)
       lms_error();
    //set TX gain
    if (LMS_SetNormalizedGain(p_device, LMS_CH_TX, *p_channel_out, 0.7) != 0)
        lms_error();
    //calibrate Tx, continue on failure
    LMS_Calibrate(p_device, LMS_CH_TX, *p_channel_out, *p_sample_rate, 0);

    tx_stream.channel = *p_channel_out;                  //channel number
    //x_stream.fifoSize = 256*1024;          //fifo size in samples
    tx_stream.fifoSize = 1024*1024;
    tx_stream.throughputVsLatency = 1;    //0 min latency, 1 max throughput
    tx_stream.dataFmt = lms_stream_t::LMS_FMT_F32; //floating point samples
    tx_stream.isTx = true;                  //TX channel
    if (LMS_SetupStream(p_device, &tx_stream) != 0)
        lms_error();

    rx_stream.channel = *p_channel_in; //channel number
    rx_stream.fifoSize = 1024 * 1024; //fifo size in samples //задавать эьт настройки
    rx_stream.throughputVsLatency = 1.0; //optimize for max throughput??????
    rx_stream.isTx = false; //RX channel
    rx_stream.dataFmt = lms_stream_t::LMS_FMT_F32; //32-bit floats
    if (LMS_SetupStream(p_device, &rx_stream) != 0)
          lms_error();

    LMS_StartStream(&rx_stream);
    LMS_StartStream(&tx_stream);
    qDebug() << "lms_config_RXTX_stream";
    return ( EXIT_SUCCESS );
}







void lms_write_TX(){
    //int ret = LMS_SendStream(&tx_stream, tx_buffer, 1024, nullptr, 1000);
    //qDebug() << "send:" << ret << endl;
}

/*double *lms_read_RX(int buff_size){
   // double *buff = new double[buff_size*2];
   // int samplesRead = LMS_RecvStream(&rx_stream, buff, buff_size-1, NULL, 1000);
    int samplesRead = LMS_RecvStream(&rx_stream, buff, buff_size-1, NULL, 1000);
    qDebug() <<" samplesRead:" << samplesRead << endl ;
    return buff;
}*/





