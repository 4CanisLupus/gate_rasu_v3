#ifndef WS_SIGNAL_H
#define WS_SIGNAL_H

#include <QObject>

class WS_signal: public QObject
{
    Q_OBJECT
public:
        explicit WS_signal(QObject *parent = nullptr);
    WS_signal(int8_t sigT, QString sigN, uint16_t sigHW, uint16_t sigQW, float sigSA);
    WS_signal(int8_t sigT, QString sigN, uint16_t sigHW, uint16_t sigQW, int32_t sigSDI);

public:
    int8_t sigType;    //тип сигнала ANA, Discr(BIN), INT
    QString sigName;     //имя сигнала
    uint16_t sigHoldW;      //номер holding регистра со значением
    uint16_t sigQualW;      //номер holding регистра с качеством сигнала
    float sigAValue;      //величина в регистре
    int32_t sigDValue;        //величина в регистре
    int32_t sigINTValue;  //величина в регистре
    int32_t sigQualValue;  // величина качества
    float sigSimA;         // величина симуляции
    int32_t sigSimD;
    int32_t sigSimInt;
};


#endif // WS_SIGNAL_H
