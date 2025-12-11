#include "include/ws_signal.h"

WS_signal::WS_signal(QObject *parent) : QObject(parent)
{
//    uint8_t sigType = 10;
//    QString sigName = " ";
//    uint16_t sigHoldW = 0;
//    uint16_t sigQualW = 0;
//    float sigAValue = 0.0;
//    int32_t sigDValue = 0;
//    int32_t sigINTValue = 0;
//    int32_t sigQualValue = 0;
//    float sigSimA = 0.0;
//    int32_t sigSimD = 0;
//    int32_t sigSimInt = 0;
}

WS_signal::WS_signal(int8_t sigT, QString sigN, uint16_t sigHW, uint16_t sigQW, float sigSA)
{
     sigType = sigT;
     sigName = sigN;
     sigHoldW = sigHW;
     sigQualW = sigQW;
     sigAValue = 0.0;
     sigQualValue = 0;
     sigSimA = sigSA;
}

WS_signal::WS_signal(int8_t sigT, QString sigN, uint16_t sigHW, uint16_t sigQW, int32_t sigSDI)
{
     sigType = sigT;
     sigName = sigN;
     sigHoldW = sigHW;
     sigQualW = sigQW;
     sigAValue = 0.0;
     sigDValue = 0;
     sigINTValue = 0;
     sigQualValue = 0;
     sigSimA = 0.0;
     sigSimD = sigSDI;
     sigSimInt = sigSDI;
}
